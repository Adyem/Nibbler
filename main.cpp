#include "GameEngine.hpp"
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

// Function prototypes
int parseArguments(int argc, char** argv, int& width, int& height);
void printUsage(const char* programName);

int main(int argc, char** argv)
{


    int width, height;

    // Parse command line arguments
    if (parseArguments(argc, argv, width, height) != 0) {
        return 1;
    }

    // Create and initialize game engine
    GameEngine engine(width, height);

    int initResult = engine.initialize();
    if (initResult != 0) {
        const char* error = engine.getError();
        if (error) {
            std::cerr << "Error: " << error << std::endl;
        } else {
            std::cerr << "Error: Unknown initialization error" << std::endl;
        }
        return 1;
    }

    // Run the game
    engine.run();

    return 0;
}

int parseArguments(int argc, char** argv, int& width, int& height) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse width
    try {
        width = std::stoi(argv[1]);
    } catch (const std::exception&) {
        std::cerr << "Error: Width must be a valid number" << std::endl;
        return 1;
    }

    // Parse height
    try {
        height = std::stoi(argv[2]);
    } catch (const std::exception&) {
        std::cerr << "Error: Height must be a valid number" << std::endl;
        return 1;
    }

    // Validate dimensions
    if (width < 10 || width > 100) {
        std::cerr << "Error: Width must be between 10 and 100" << std::endl;
        return 1;
    }

    if (height < 10 || height > 100) {
        std::cerr << "Error: Height must be between 10 and 100" << std::endl;
        return 1;
    }

    return 0;
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <width> <height>" << std::endl;
    std::cout << "  width:  Game area width (10-100)" << std::endl;
    std::cout << "  height: Game area height (10-100)" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Arrow keys: Move snake" << std::endl;
    std::cout << "  1, 2, 3:    Switch graphics libraries" << std::endl;
    std::cout << "  ESC:        Quit game" << std::endl;
}


