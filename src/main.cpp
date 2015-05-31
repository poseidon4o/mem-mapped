#include "mfile.h"
#include <iostream>
#include <chrono>
#include <random>

using namespace std;
using namespace std::chrono;

random_device dev;
mt19937 rnd(dev());

uniform_int_distribution<uint16_t> rndChar(0, 255);


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
    file[c] = rndChar(rnd) + c;
}
END_TEST

START_TEST(backward_write)
uint8_t x;
for (uint64_t c = fileSize - 1; c > 0; --c) {
    file[c] = rndChar(rnd) + c;
}
END_TEST

START_TEST(stride_forward_write)
for (int r = 0; r < 10; ++r) {
    for (uint64_t c = 0; c + r < fileSize; c += MemoryMapped::pageSize) {
        file[c + r] = rndChar(rnd) + c;
    }
}
END_TEST


START_TEST(stride_backward_write)
uint8_t x;
for (int r = 0; r < 10; ++r) {
    for (uint64_t c = file.size() - 1; c - r > 0 && c - r < fileSize; c -= MemoryMapped::pageSize) {
        file[c - r] = rndChar(rnd) + c;
    }
}
END_TEST



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
    file[dist(rnd)] = rndChar(rnd) + c;
}
END_TEST


START_TEST(sequential_move)
// seems random but utilizes the paging to it's best potential
for (int c = 0; c < MemoryMapped::pageSize * 10; ++c) {
    for (int r = 0; r < MemoryMapped::pageCount / 2; ++r) {
        file[r * MemoryMapped::pageSize + c] = (uint8_t&)file[r * MemoryMapped::pageSize + c + (MemoryMapped::pageCount / 2 + 1) * (MemoryMapped::pageSize * 10)];
    }
}
END_TEST

START_TEST(breaking_move)
// worst case for fixed position paging
for (int c = MemoryMapped::pageSize; c < fileSize; c += MemoryMapped::pageSize) {
    file[c] = (uint8_t&)file[c - 1];
}
END_TEST

START_TEST(random_move)
uniform_int_distribution<uint64_t> dist(0, fileSize - 1);
for (int c = 0; c < 100000; ++c) {
    file[dist(rnd)] = (uint8_t&)file[dist(rnd)];
}
END_TEST


int main(int argc, char * argv[]) {
    try {
        //test_forward_read();
        //test_forward_write();

        //test_backward_read();
        //test_backward_write();

        //test_stride_forward_read();
        //test_stride_forward_write();

        //test_stride_backward_read();
        //test_stride_backward_write();

        //test_random_read_100000();
        //test_random_write_100000();

        //test_sequential_move();
        //test_random_move();
        //test_breaking_move();

        if (argc != 3) {
            std::cout << "wrong arguments";
            return -1;
        }
        MemoryMapped mfile(argv[1]);
        auto start = high_resolution_clock::now();
        for (int c = 0; c < mfile.size(); ++c) {
            mfile[c] = argv[2][0];
        }
        auto end = high_resolution_clock::now();
        std::cout << duration_cast<milliseconds>(end - start).count();
    } catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
