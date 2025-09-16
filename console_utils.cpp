#include "console_utils.hpp"
#include <iostream>

// ANSI color codes for red and yellow; reset at end
static constexpr const char *RED = "\033[31m";
static constexpr const char *YELLOW = "\033[33m";
static constexpr const char *RESET = "\033[0m";

void print_error(const std::string &msg) {
    std::cerr << RED << msg << RESET << std::endl;
}

void print_warning(const std::string &msg) {
    std::cerr << YELLOW << msg << RESET << std::endl;
}

