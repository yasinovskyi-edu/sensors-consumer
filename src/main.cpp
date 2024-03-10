#include "mystring.h"
#include "point.h"
#include "threadsafequeue.h"

#include <sys/stat.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>
#include <variant>

using SensorData = std::variant<Point, MyString>;

class Sensor
{
protected:
    ThreadSafeQueue<SensorData> &dataQueue_;
    std::uniform_int_distribution<> disFreq_;
    std::mt19937 gen_;

    virtual void generateData() = 0;

public:
    Sensor(ThreadSafeQueue<SensorData> &dataQueue, int minFreq, int maxFreq)
        : dataQueue_(dataQueue), disFreq_(minFreq, maxFreq), gen_(std::random_device{}())
    {
    }

    void run()
    {
        while (true)
        {
            generateData();
            dataQueue_.push(data_);
            sleepRandomly();
        }
    }

private:
    void sleepRandomly()
    {
        int frequency = disFreq_(gen_);
        std::chrono::milliseconds interval(1000 / frequency);
        std::this_thread::sleep_for(interval);
    }

protected:
    SensorData data_;
};

class PointSensor : public Sensor
{
public:
    PointSensor(ThreadSafeQueue<SensorData> &dataQueue) : Sensor(dataQueue, 30, 40) {}

private:
    void generateData() override { data_ = Point::createRandom(gen_); }
};

class StringSensor : public Sensor
{
public:
    StringSensor(ThreadSafeQueue<SensorData> &dataQueue) : Sensor(dataQueue, 40, 50) {}

private:
    void generateData() override { data_ = MyString::createRandom(gen_); }
};

class Consumer
{
private:
    ThreadSafeQueue<SensorData> &dataQueue_;
    std::ofstream &pipeWriter_;
    std::mt19937 gen_;

public:
    Consumer(ThreadSafeQueue<SensorData> &dataQueue, std::ofstream &pipeWriter)
        : dataQueue_(dataQueue), pipeWriter_(pipeWriter), gen_(std::random_device{}())
    {
    }

    void run()
    {
        while (true)
        {
            SensorData data;
            dataQueue_.wait_and_pop(data);

            processData(data);
            sleepRandomly();
        }
    }

private:
    void processData(const SensorData &data)
    {
        std::visit(
            [&](auto &data)
            {
                using T = std::decay_t<decltype(data)>;
                if constexpr (std::is_same_v<T, Point>)
                {
                    std::cout << "Point: (" << static_cast<int>(data.x) << ", "
                              << static_cast<int>(data.y) << ")" << std::endl;
                }
                else if constexpr (std::is_same_v<T, MyString>)
                {
                    std::cout << "MyString: size=" << static_cast<int>(data.length()) << ", data='"
                              << data.data() << "'" << std::endl;
                }
                data.write(pipeWriter_);
            },
            data);
    }

    void sleepRandomly()
    {
        std::uniform_int_distribution<> disFreq(10, 50);
        int frequency = disFreq(gen_);
        std::chrono::milliseconds interval(1000 / frequency);
        std::this_thread::sleep_for(interval);
    }
};

int main(int, char **)
{
    const std::string pipeName = "/tmp/sensor_data";
    std::remove(pipeName.c_str());
    mkfifo(pipeName.c_str(), 0666);

    std::ofstream pipeWriter(pipeName, std::ios::out | std::ios::binary);
    if (!pipeWriter.is_open())
    {
        std::cerr << "Failed to open named pipe for writing." << std::endl;
        return 1;
    }

    ThreadSafeQueue<SensorData> dataQueue;

    PointSensor pointSensor(dataQueue);
    StringSensor stringSensor(dataQueue);
    Consumer consumer(dataQueue, pipeWriter);

    std::thread t1(&Sensor::run, &pointSensor);
    std::thread t2(&Sensor::run, &stringSensor);
    std::thread t3(&Consumer::run, &consumer);

    t1.join();
    t2.join();
    t3.join();

    pipeWriter.close();
    std::remove(pipeName.c_str());

    return 0;
}
