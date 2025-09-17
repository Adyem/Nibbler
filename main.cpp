#include "GameEngine.hpp"
#include "console_utils.hpp"
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <cctype>

// Function prototypes
int parseArguments(int argc, char** argv, int& width, int& height, std::string &bonusMap);
void printUsage(const char* programName);
int selectGraphicsLibrary();

#ifndef NIBBLER_NO_MAIN
int main(int argc, char** argv) {
    int width = 30, height = 30; // defaults
    std::string bonusMap;

    if (parseArguments(argc, argv, width, height, bonusMap) != 0) {
        return 1;
    }

    // Welcome message
    std::cout << "\n🐍 Welcome to NIBBLER - Multi-Graphics Snake Game! 🐍" << std::endl;
    std::cout << "Seamless switching between 4 different graphics libraries!" << std::endl;

    // Let user select graphics library
    int selectedLibrary = selectGraphicsLibrary();
    if (selectedLibrary < 0) {
        return 1;
    }

    // Create and initialize game engine
    GameEngine engine(width, height);
    if (!bonusMap.empty()) {
        if (engine.loadBonusMap(bonusMap.c_str()) != 0) {
            print_error(std::string("Error: ") + (engine.getError() ? engine.getError() : "Unknown bonus map error"));
            return 1;
        }
    }

    int initResult = engine.initialize(selectedLibrary);
    if (initResult != 0) {
        const char* error = engine.getError();
        if (error)
            print_error(std::string("Error: ") + error);
        else
            print_error("Error: Unknown initialization error");
        return 1;
    }

    // Run the game
    engine.run();

    return 0;
}
#endif // NIBBLER_NO_MAIN

static bool validateBonusFile(const std::string &path) {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::file_status st = fs::status(path, ec);
    if (ec) {
        print_error(std::string("Error: Cannot access '") + path + "' (" + ec.message() + ")");
        return false;
    }
    if (!fs::exists(st)) {
        print_error(std::string("Error: File '") + path + "' does not exist");
        return false;
    }
    if (!fs::is_regular_file(st)) {
        print_error(std::string("Error: '") + path + "' is not a regular file");
        return false;
    }
    // Extension check (.nib, case-insensitive)
    auto dot = path.rfind('.');
    if (dot == std::string::npos) {
        print_error("Error: Bonus map must have .nib extension");
        return false;
    }
    std::string ext = path.substr(dot);
    for (char &c : ext) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (ext != ".nib") {
        print_error("Error: Bonus map must use .nib extension");
        return false;
    }
    std::ifstream f(path.c_str());
    if (!f.good()) {
        print_error(std::string("Error: Cannot open '") + path + "' for reading");
        return false;
    }
    return true;
}

int parseArguments(int argc, char** argv, int& width, int& height, std::string &bonusMap) {
    // Modes:
    // 1) ./nibbler <width> <height>
    // 2) ./nibbler -b <mapfile.nib>
    if (argc == 3 && std::string(argv[1]) == "-b") {
        std::string candidate = argv[2];
        if (!validateBonusFile(candidate)) {
            return 1;
        }
        bonusMap = candidate;
        return 0; // width/height defaults; map sets game parameters later
    }
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }
    try { width = std::stoi(argv[1]); } catch (...) { print_error("Error: Width must be a valid number"); return 1; }
    try { height = std::stoi(argv[2]); } catch (...) { print_error("Error: Height must be a valid number"); return 1; }

    const int MIN_DIM = 10;
    const int MAX_DIM = 30; // project constraint: max width/height is 30

    if (width < MIN_DIM || width > MAX_DIM) {
        print_error("Error: Width must be between 10 and 30");
        return 1;
    }
    if (height < MIN_DIM || height > MAX_DIM) {
        print_error("Error: Height must be between 10 and 30");
        return 1;
    }
    return 0;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <width> <height>" << std::endl;
    std::cout << "   or:  " << programName << " -b <gamemodefile.nib>" << std::endl;
    std::cout << "Notes for -b mode:" << std::endl;
    std::cout << "  * File must exist, be readable, regular, and end with .nib" << std::endl;
    std::cout << "  width:  Game area width (10-30)" << std::endl;
    std::cout << "  height: Game area height (10-30)" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Arrow keys: Move snake" << std::endl;
    std::cout << "  1, 2, 3, 4: Switch graphics libraries" << std::endl;
    std::cout << "  ESC:        Quit game" << std::endl;
}

int selectGraphicsLibrary() {
    std::cout << "\n=== NIBBLER - Graphics Library Selection ===" << std::endl;
    std::cout << "Choose your preferred graphics library:" << std::endl;
    std::cout << "  1. NCurses (Terminal-based)" << std::endl;
    std::cout << "  2. SDL2 (Window-based)" << std::endl;
    std::cout << "  3. OpenGL (Window-based)" << std::endl;
    std::cout << "  4. Raylib (Window-based)" << std::endl;
    std::cout << "Enter your choice (1-4): ";

    std::string input;
    std::getline(std::cin, input);

    if (input.empty()) {
        std::cout << "No selection made. Defaulting to NCurses." << std::endl;
        return 0;
    }

    try {
        int choice = std::stoi(input);
        if (choice >= 1 && choice <= 4) {
            const char* libNames[] = {"NCurses", "SDL2", "OpenGL", "Raylib"};
            std::cout << "Selected: " << libNames[choice - 1] << std::endl;
            return choice - 1; // Convert to 0-based index
        } else {
            std::cout << "Invalid choice. Defaulting to NCurses." << std::endl;
            return 0;
        }
    } catch (const std::exception&) {
        std::cout << "Invalid input. Defaulting to NCurses." << std::endl;
        return 0;
    }
}
