#include "mfile.h"
#include <iostream>

int main() {
    try {
        MemoryMapped file("data.dat");

        for (uint64_t c = 0; c < file.size(); ++c) {
            file[c] = 'r';
        }

        file.flush();
    } catch (std::exception & e) {
        std::cerr << e.what();
    }

    std::cin.get();

    return 0;
}
