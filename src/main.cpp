#include "mfile.h"
#include <iostream>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;



#define START_TEST(MESSAGE)\
void test_##MESSAGE()\
{\
    const char * message = "" #MESSAGE "";\
    MemoryMapped file("data.dat");\
    auto start = high_resolution_clock::now();\
    auto fileSize = file.size();

#define END_TEST\
    file.flush();\
    auto end = high_resolution_clock::now();\
    auto dur = duration_cast<milliseconds>(end - start).count();\
    std::cout << message << " time: " << dur << "ms" << std::endl;\
}



START_TEST(forward_read)
uint8_t x;
for (uint64_t c = 0; c < fileSize; ++c) {
    x = file[c];
}
END_TEST

START_TEST(backward_read)
uint8_t x;
for (uint64_t c = fileSize - 1; c > 0; --c) {
    x = file[c];
}
END_TEST

START_TEST(stride_forward_read)
uint8_t x;
for (int r = 0; r < 10; ++r) {
    for (uint64_t c = 0; c + r < fileSize; c += MemoryMapped::pageSize) {
        x = file[c + r];
    }
}
END_TEST


START_TEST(stride_backward_read)
uint8_t x;
for (int r = 0; r < 10; ++r) {
    for (uint64_t c = file.size() - 1; c - r > 0 && c - r < fileSize; c -= MemoryMapped::pageSize) {
        x = file[c - r];
    }
}
END_TEST




START_TEST(forward_write)
for (uint64_t c = 0; c < fileSize; ++c) {
    file[c] = c % 255;
}
END_TEST

START_TEST(backward_write)
uint8_t x;
for (uint64_t c = fileSize - 1; c > 0; --c) {
    file[c] = c % 255;
}
END_TEST

START_TEST(stride_forward_write)
for (int r = 0; r < 10; ++r) {
    for (uint64_t c = 0; c + r < fileSize; c += MemoryMapped::pageSize) {
        file[c + r] = c % 255;
    }
}
END_TEST


START_TEST(stride_backward_write)
uint8_t x;
for (int r = 0; r < 10; ++r) {
    for (uint64_t c = file.size() - 1; c - r > 0 && c - r < fileSize; c -= MemoryMapped::pageSize) {
        file[c - r] = c % 255;
    }
}
END_TEST


random_device dev;
mt19937 rnd(dev());

START_TEST(random_read_100000)
uniform_int_distribution<uint64_t> dist(0, fileSize - 1);
uint64_t x;
for (uint64_t c = 0; c < 100000; ++c) {
    x = file[dist(rnd)];
}
END_TEST

START_TEST(random_write_100000)
uniform_int_distribution<uint64_t> dist(0, fileSize - 1);
for (uint64_t c = 0; c < 100000; ++c) {
    file[dist(rnd)] = c % 255;
}
END_TEST



int main() {
    try {
        test_forward_read();
        test_backward_read();

        test_stride_forward_read();
        test_stride_backward_read();
        
        test_forward_write();
        test_backward_write();

        test_stride_forward_write();
        test_stride_backward_write();

        test_random_read_100000();
        test_random_write_100000();

    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }

    std::cout << "done";

    return 0;
}
