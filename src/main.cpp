#include "mfile.h"
#include <iostream>

int main() {
    try {
        MemoryMapped file("C:/tmp/sample.avi");

        for (uint64_t c = 0; c < file.size(); ++c) {
            file[c] = 0;
        }

        file.flush();
    } catch (std::exception & e) {
        std::cerr << e.what();
    }

    std::cin.get();

    return 0;
}
