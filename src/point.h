#include <cstdint>
#include <fstream>
#include <random>

struct Point
{
    uint8_t x, y;

    static Point createRandom(std::mt19937 &gen)
    {
        std::uniform_int_distribution<> dis(0, 255);
        return { static_cast<uint8_t>(dis(gen)), static_cast<uint8_t>(dis(gen)) };
    }

    void write(std::ofstream &pipeWriter) const
    {
        pipeWriter.put(0); // Tag for Point data
        pipeWriter.write(reinterpret_cast<const char *>(&x), sizeof(x));
        pipeWriter.write(reinterpret_cast<const char *>(&y), sizeof(y));
    }
};
