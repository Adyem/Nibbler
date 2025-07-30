#include "tests.hpp"
#include "game_data.hpp"
#include <iostream>


int main() {
    if (test_game_data() != 0) {
        std::cerr << "Tests failed" << std::endl;
        return 1;
    }
    std::cout << "All tests passed" << std::endl;
    return 0;
}
