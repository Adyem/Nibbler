#include "ConsoleGraphics.hpp"
#include "../game_data.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>

ConsoleGraphics::ConsoleGraphics()
    : _initialized(false), _shouldContinue(true) {
}

ConsoleGraphics::~ConsoleGraphics() {
    if (_initialized) {
        shutdown();
    }
}

int ConsoleGraphics::initialize() {
    if (_initialized) {
        return 0;
    }

    // Save current terminal settings
    if (tcgetattr(STDIN_FILENO, &_originalTermios) != 0) {
        setError("Failed to get terminal attributes");
        return 1;
    }

    // Set non-blocking input
    setNonBlockingInput();

    // Clear screen and hide cursor
    clearScreen();
    std::cout << "\033[?25l"; // Hide cursor
    std::cout.flush();

    _initialized = true;
    return 0;
}

void ConsoleGraphics::shutdown() {
    if (!_initialized) {
        return;
    }

    // Restore terminal settings
    restoreTerminalSettings();

    // Show cursor and clear screen
    std::cout << "\033[?25h"; // Show cursor
    clearScreen();
    std::cout.flush();

    _initialized = false;
}

void ConsoleGraphics::render(const game_data& game) {
    if (!_initialized) {
        return;
    }

    // Move cursor to top-left
    setCursorPosition(0, 0);

    size_t width = game.get_width();
    size_t height = game.get_height();

    // Draw top border
    std::cout << "+";
    for (size_t x = 0; x < width; ++x) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;

    // Draw game area
    for (size_t y = 0; y < height; ++y) {
        std::cout << "|";
        for (size_t x = 0; x < width; ++x) {
            char ch = getCharFromGameTile(static_cast<int>(x), static_cast<int>(y), game);
            std::cout << ch;
        }
        std::cout << "|" << std::endl;
    }

    // Draw bottom border
    std::cout << "+";
    for (size_t x = 0; x < width; ++x) {
        std::cout << "-";
    }
    std::cout << "+" << std::endl;

    // Display game info
    std::cout << "Snake Length: " << game.get_snake_length(0) << std::endl;

    // Check if game has started (snake is moving)
    bool gameStarted = (game.get_direction_moving(0) != DIRECTION_NONE);

    if (!gameStarted) {
        std::cout << ">>> PRESS ANY ARROW KEY TO START THE GAME <<<" << std::endl;
    } else {
        std::cout << "Controls: Arrow keys to move, 1/2/3 to switch graphics, ESC to quit" << std::endl;
    }

    std::cout.flush();
}

GameKey ConsoleGraphics::getInput() {
    if (!_initialized) {
        return GameKey::NONE;
    }

    char ch;
    if (read(STDIN_FILENO, &ch, 1) <= 0) {
        return GameKey::NONE;
    }

    // Handle escape sequences (arrow keys)
    if (ch == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) <= 0) return GameKey::NONE;
        if (read(STDIN_FILENO, &seq[1], 1) <= 0) return GameKey::NONE;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return GameKey::UP;
                case 'B': return GameKey::DOWN;
                case 'C': return GameKey::RIGHT;
                case 'D': return GameKey::LEFT;
            }
        }
        return GameKey::ESCAPE;
    }

    // Handle regular keys
    switch (ch) {
        case '1': return GameKey::KEY_1;
        case '2': return GameKey::KEY_2;
        case '3': return GameKey::KEY_3;
        case 27:  return GameKey::ESCAPE;  // ESC key
        case 'q':
        case 'Q': return GameKey::QUIT;
        default:  return GameKey::NONE;
    }
}

const char* ConsoleGraphics::getName() const {
    return "Console Graphics";
}

bool ConsoleGraphics::shouldContinue() const {
    return _shouldContinue;
}

const char* ConsoleGraphics::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

// Private helper methods
void ConsoleGraphics::clearScreen() {
    std::cout << "\033[2J"; // Clear entire screen
}

void ConsoleGraphics::setCursorPosition(int x, int y) {
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
}

void ConsoleGraphics::setNonBlockingInput() {
    struct termios newTermios = _originalTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    newTermios.c_cc[VMIN] = 0;
    newTermios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);

    // Set stdin to non-blocking
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void ConsoleGraphics::restoreTerminalSettings() {
    tcsetattr(STDIN_FILENO, TCSANOW, &_originalTermios);

    // Remove non-blocking flag
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

char ConsoleGraphics::getCharFromGameTile(int x, int y, const game_data& game) {
    // Check layer 2 first (snake and food)
    int layer2Value = game.get_map_value(x, y, 2);
    if (layer2Value == FOOD) {
        return '*';  // Food
    } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
        return '@';  // Snake head
    } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
        return 'o';  // Snake body
    }

    // Check layer 0 (terrain)
    int layer0Value = game.get_map_value(x, y, 0);
    if (layer0Value == GAME_TILE_WALL) {
        return '#';  // Wall
    } else if (layer0Value == GAME_TILE_ICE) {
        return '~';  // Ice
    }

    return ' ';  // Empty space
}

void ConsoleGraphics::setError(const std::string& error) {
    _errorMessage = error;
}

// C interface for dynamic library loading
extern "C" {
    IGraphicsLibrary* createGraphicsLibrary() {
        return new ConsoleGraphics();
    }

    void destroyGraphicsLibrary(IGraphicsLibrary* lib) {
        delete lib;
    }

    const char* getLibraryName() {
        return "Console Graphics Library";
    }

    const char* getLibraryVersion() {
        return "1.0.0";
    }
}