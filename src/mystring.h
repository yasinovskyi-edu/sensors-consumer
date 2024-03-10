#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>

class MyString
{
private:
    char *data_;
    uint8_t length_;

    void copyFrom(const char *src, uint8_t len)
    {
        data_ = new char[len + 1];
        std::memcpy(data_, src, len);
        data_[len] = '\0';
        length_ = len;
    }

    void moveFrom(MyString &&other)
    {
        data_ = other.data_;
        length_ = other.length_;
        other.data_ = nullptr;
        other.length_ = 0;
    }

    void freeData()
    {
        delete[] data_;
        data_ = nullptr;
        length_ = 0;
    }

public:
    MyString() : data_(nullptr), length_(0) {}

    MyString(const MyString &other) : MyString(other.data_, other.length_) {}

    MyString(MyString &&other) noexcept { moveFrom(std::move(other)); }

    MyString &operator=(const MyString &other)
    {
        if (this != &other)
        {
            freeData();
            copyFrom(other.data_, other.length_);
        }
        return *this;
    }

    MyString &operator=(MyString &&other) noexcept
    {
        if (this != &other)
        {
            freeData();
            moveFrom(std::move(other));
        }
        return *this;
    }

    ~MyString() { freeData(); }

    static MyString createRandom(std::mt19937 &gen)
    {
        std::uniform_int_distribution<> disChar(33, 126); // Printable ASCII characters
        std::uniform_int_distribution<> disLength(0, 255);

        uint8_t length = disLength(gen);
        char *data = new char[length + 1];
        std::generate(data, data + length, [&]() { return disChar(gen); });
        data[length] = '\0';

        MyString result(data, length);

        delete[] data;

        return result;
    }

    const char *data() const { return data_; }

    uint8_t length() const { return length_; }

    void write(std::ofstream &pipeWriter) const
    {
        pipeWriter.put(1); // Tag for String data
        pipeWriter.write(reinterpret_cast<const char *>(&length_), sizeof(length_));
        pipeWriter.write(data_, length_);
    }

private:
    MyString(const char *data, uint8_t length) { copyFrom(data, length); }
};
