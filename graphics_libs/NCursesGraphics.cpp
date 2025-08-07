#include "NCursesGraphics.hpp"
#include "../game_data.hpp"
#include <cstdlib>

NCursesGraphics::NCursesGraphics()
    : _initialized(false), _shouldContinue(true), _frameRate(60),
      _gameWindow(nullptr), _infoWindow(nullptr) {
    clearError();
}

NCursesGraphics::~NCursesGraphics() {
    if (_initialized) {
        shutdown();
    }
}

int NCursesGraphics::initialize() {
    if (_initialized) {
        return 0;
    }

    // Initialize ncurses
    if (!initscr()) {
        setError("Failed to initialize ncurses");
        return 1;
    }

    // Check if terminal supports colors
    if (!has_colors()) {
        endwin();
        setError("Terminal does not support colors");
        return 1;
    }

    // Configure ncurses
    start_color();
    cbreak();           // Disable line buffering
    noecho();           // Don't echo pressed keys
    keypad(stdscr, TRUE); // Enable special keys
    nodelay(stdscr, TRUE); // Make getch() non-blocking
    curs_set(0);        // Hide cursor

    // Initialize color pairs
    initializeColors();



    // Clear screen
    clear();
    refresh();

    _initialized = true;
    return 0;
}

void NCursesGraphics::shutdown() {
    if (!_initialized) {
        return;
    }

    // Clean up windows
    if (_gameWindow) {
        delwin(_gameWindow);
        _gameWindow = nullptr;
    }
    if (_infoWindow) {
        delwin(_infoWindow);
        _infoWindow = nullptr;
    }

    // Restore terminal
    endwin();
    _initialized = false;
}

void NCursesGraphics::render(const game_data& game) {
    if (!_initialized) {
        return;
    }

    // Clear screen
    clear();

    // Get terminal size
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    size_t gameWidth = game.get_width();
    size_t gameHeight = game.get_height();

    // Calculate centering offsets
    int startY = (termHeight - static_cast<int>(gameHeight) - 4) / 2; // -4 for borders and info
    int startX = (termWidth - static_cast<int>(gameWidth) - 2) / 2;   // -2 for borders

    // Ensure we don't go negative
    if (startY < 0) startY = 0;
    if (startX < 0) startX = 0;

    // Draw game border
    attron(COLOR_PAIR(COLOR_BORDER));

    // Top border
    mvhline(startY, startX, ACS_HLINE, static_cast<int>(gameWidth) + 2);
    mvaddch(startY, startX, ACS_ULCORNER);
    mvaddch(startY, startX + static_cast<int>(gameWidth) + 1, ACS_URCORNER);

    // Side borders and game area
    for (size_t y = 0; y < gameHeight; ++y) {
        mvaddch(startY + 1 + static_cast<int>(y), startX, ACS_VLINE);
        mvaddch(startY + 1 + static_cast<int>(y), startX + static_cast<int>(gameWidth) + 1, ACS_VLINE);

        // Draw game tiles
        for (size_t x = 0; x < gameWidth; ++x) {
            char ch = getCharFromGameTile(static_cast<int>(x), static_cast<int>(y), game);
            int colorPair = getColorFromGameTile(static_cast<int>(x), static_cast<int>(y), game);

            attron(COLOR_PAIR(colorPair));
            mvaddch(startY + 1 + static_cast<int>(y), startX + 1 + static_cast<int>(x), ch);
            attroff(COLOR_PAIR(colorPair));
        }
    }

    // Bottom border
    attron(COLOR_PAIR(COLOR_BORDER));
    mvhline(startY + static_cast<int>(gameHeight) + 1, startX, ACS_HLINE, static_cast<int>(gameWidth) + 2);
    mvaddch(startY + static_cast<int>(gameHeight) + 1, startX, ACS_LLCORNER);
    mvaddch(startY + static_cast<int>(gameHeight) + 1, startX + static_cast<int>(gameWidth) + 1, ACS_LRCORNER);
    attroff(COLOR_PAIR(COLOR_BORDER));

    // Draw game info
    drawInfo(game);

    // Refresh screen
    refresh();
}

GameKey NCursesGraphics::getInput() {
    if (!_initialized) {
        return GameKey::NONE;
    }

    int ch = getch();

    switch (ch) {
        case ERR:           // No input available
            return GameKey::NONE;
        case KEY_UP:
            return GameKey::UP;
        case KEY_DOWN:
            return GameKey::DOWN;
        case KEY_LEFT:
            return GameKey::LEFT;
        case KEY_RIGHT:
            return GameKey::RIGHT;
        case '1':
            return GameKey::KEY_1;
        case '2':
            return GameKey::KEY_2;
        case '3':
            return GameKey::KEY_3;
        case 27:            // ESC key
        case 'q':
        case 'Q':
            return GameKey::ESCAPE;
        default:
            return GameKey::NONE;
    }
}

const char* NCursesGraphics::getName() const {
    return "NCurses Graphics";
}

bool NCursesGraphics::shouldContinue() const {
    return _shouldContinue;
}

void NCursesGraphics::setFrameRate(int fps) {
    if (fps > 0 && fps <= 120) {
        _frameRate = fps;
    }
}

const char* NCursesGraphics::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

// Private helper methods
void NCursesGraphics::initializeColors() {
    // Initialize color pairs
    init_pair(COLOR_SNAKE_HEAD, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_SNAKE_BODY, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_FOOD, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_WALL, COLOR_WHITE, COLOR_WHITE);
    init_pair(COLOR_ICE, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_BORDER, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_INFO, COLOR_MAGENTA, COLOR_BLACK);
}

void NCursesGraphics::drawInfo(const game_data& game) {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw info at the bottom
    attron(COLOR_PAIR(COLOR_INFO));

    // Snake length
    mvprintw(termHeight - 4, 2, "Snake Length: %d", game.get_snake_length(0));

    // Check if game has started (snake is moving)
    bool gameStarted = (game.get_direction_moving(0) != 0);

    if (!gameStarted) {
        // Show start message
        attron(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
        mvprintw(termHeight - 3, 2, ">>> PRESS ANY ARROW KEY TO START THE GAME <<<");
        attroff(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
    } else {
        // Show normal controls
        mvprintw(termHeight - 3, 2, "Controls: Arrow keys=Move, 1/2/3=Switch graphics, ESC/Q=Quit");
    }

    // Library name
    mvprintw(termHeight - 2, 2, "Graphics: %s (60 FPS)", getName());

    // Additional info
    mvprintw(termHeight - 1, 2, "Nibbler - Snake Game with Dynamic Libraries");

    attroff(COLOR_PAIR(COLOR_INFO));
}

char NCursesGraphics::getCharFromGameTile(int x, int y, const game_data& game) {
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

int NCursesGraphics::getColorFromGameTile(int x, int y, const game_data& game) {
    // Check layer 2 first (snake and food)
    int layer2Value = game.get_map_value(x, y, 2);
    if (layer2Value == FOOD) {
        return COLOR_FOOD;
    } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
        return COLOR_SNAKE_HEAD;
    } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
        return COLOR_SNAKE_BODY;
    }

    // Check layer 0 (terrain)
    int layer0Value = game.get_map_value(x, y, 0);
    if (layer0Value == GAME_TILE_WALL) {
        return COLOR_WALL;
    } else if (layer0Value == GAME_TILE_ICE) {
        return COLOR_ICE;
    }

    return 0;  // Default color
}

void NCursesGraphics::setError(const std::string& error) {
    _errorMessage = error;
}

void NCursesGraphics::clearError() {
    _errorMessage.clear();
}

// C interface for dynamic library loading
extern "C" {
    IGraphicsLibrary* createGraphicsLibrary() {
        return new NCursesGraphics();
    }

    void destroyGraphicsLibrary(IGraphicsLibrary* lib) {
        delete lib;
    }

    const char* getLibraryName() {
        return "NCurses Graphics Library";
    }

    const char* getLibraryVersion() {
        return "1.0.0";
    }
}