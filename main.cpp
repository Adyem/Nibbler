#include "GameEngine.hpp"
#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

// Function prototypes
int parseArguments(int argc, char** argv, int& width, int& height);
void printUsage(const char* programName);
int selectGraphicsLibrary();

int main(int argc, char** argv)
{


    int width, height;

    // Parse command line arguments
    if (parseArguments(argc, argv, width, height) != 0) {
        return 1;
    }

    // Let user select graphics library
    int selectedLibrary = selectGraphicsLibrary();
    if (selectedLibrary < 0) {
        return 1;
    }

    // Create and initialize game engine
    GameEngine engine(width, height);

    int initResult = engine.initialize(selectedLibrary);
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

int selectGraphicsLibrary() {
    std::cout << "\n=== NIBBLER - Graphics Library Selection ===" << std::endl;
    std::cout << "Choose your preferred graphics library:" << std::endl;
    std::cout << "  1. NCurses (Terminal-based)" << std::endl;
    std::cout << "  2. SDL2 (Window-based)" << std::endl;
    std::cout << "  3. SFML (Window-based)" << std::endl;
    std::cout << "Enter your choice (1-3): ";

    std::string input;
    std::getline(std::cin, input);

    if (input.empty()) {
        std::cout << "No selection made. Defaulting to NCurses." << std::endl;
        return 0;
    }

    try {
        int choice = std::stoi(input);
        if (choice >= 1 && choice <= 3) {
            const char* libNames[] = {"NCurses", "SDL2", "SFML"};
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


