#include <iostream>
#include "interpreter.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "The file name was expected\n";
        return 1;
    }

    if (!interpret_file(argv[1], std::cout)) {
        std::cerr << "Interpretation failed\n";
        return 1;
    }

    return 0;
}