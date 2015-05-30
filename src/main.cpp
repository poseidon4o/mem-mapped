#include "mfile.h"
#include <iostream>
#include <chrono>

int main() {
    using namespace std::chrono;
    try {
        MemoryMapped file("data.dat");

        auto start = high_resolution_clock::now();
        uint8_t x;
        for (uint64_t c = file.size() - 1; c > 0; --c) {
            x = file[c];
        }

        file.flush();

        auto end = high_resolution_clock::now();
        auto dur = duration_cast<milliseconds>(end - start).count();
        std::cout << dur << std::endl;
    } catch (std::exception & e) {
        std::cerr << e.what();
    }

    std::cout << "done";
    std::cin.get();

    return 0;
}
