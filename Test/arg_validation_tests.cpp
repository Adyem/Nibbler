#define NIBBLER_NO_MAIN
#include "../main.cpp"
#include "../console_utils.cpp"

#include <sstream>
#include <iostream>
#include <cassert>

static void test_usage_message_for_wrong_arguments() {
    int width = 0;
    int height = 0;
    std::string bonus;

    char arg0[] = "nibbler";
    char arg1[] = "20"; // only one dimension provided
    char* argv[] = {arg0, arg1};

    std::stringstream capture;
    std::streambuf* oldCout = std::cout.rdbuf(capture.rdbuf());
    int result = parseArguments(2, argv, width, height, bonus);
    std::cout.rdbuf(oldCout);

    assert(result != 0);
    std::string output = capture.str();
    assert(output.find("Usage:") != std::string::npos);
}

static void test_error_handling_for_invalid_dimensions() {
    int width = 0;
    int height = 0;
    std::string bonus;

    char arg0[] = "nibbler";
    char arg1[] = "5";   // invalid width (too small)
    char arg2[] = "15";
    char* argv[] = {arg0, arg1, arg2};

    std::stringstream captureErr;
    std::streambuf* oldCerr = std::cerr.rdbuf(captureErr.rdbuf());
    int result = parseArguments(3, argv, width, height, bonus);
    std::cerr.rdbuf(oldCerr);

    assert(result != 0);
    std::string errOutput = captureErr.str();
    assert(errOutput.find("Width must be between 10 and 30") != std::string::npos);
}

int main() {
    test_usage_message_for_wrong_arguments();
    test_error_handling_for_invalid_dimensions();
    std::cout << "Argument validation tests passed" << std::endl;
    return 0;
}
