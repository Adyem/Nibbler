#include "NCursesGraphics.hpp"
#include "../../game_data.hpp"
#include <cstdlib>

NCursesGraphics::NCursesGraphics()
    : _initialized(false), _shouldContinue(true), _frameRate(60),
      _gameWindow(nullptr), _switchMessageTimer(0), _infoWindow(nullptr),
      _menuSystem(nullptr) {
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
    cbreak();              // Disable line buffering
    noecho();              // Don't echo pressed keys
    keypad(stdscr, TRUE);  // Enable special keys
    nodelay(stdscr, TRUE); // Make getch() non-blocking
    curs_set(0);           // Hide cursor

    // Force input focus and flush any pending input
    flushinp(); // Clear input buffer

    // Initialize color pairs
    initializeColors();

    // Clear screen and force multiple refreshes to ensure proper initialization
    clear();
    refresh();

    // Additional refresh to ensure terminal is ready for input
    doupdate();

    // Force the terminal to be ready for input
    flushinp(); // Clear any stale input
    refresh();  // Final refresh

    _initialized = true;

    // Force input readiness
    forceInputReadiness();

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

    // Detect palette change at runtime
    bool wantAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    if (wantAlt != _altColorsActive) {
        _altColorsActive = wantAlt;
        initializeColors();
    }

    // Clear screen
    clear();

    // Check if we should render menu instead of game
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        renderMenu(game);
        refresh();
        return;
    }

    // Get terminal size
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    size_t gameWidth = game.get_width();
    size_t gameHeight = game.get_height();

    // Calculate centering offsets
    int startY = (termHeight - static_cast<int>(gameHeight) - 4) / 2; // -4 for borders and info
    int startX = (termWidth - static_cast<int>(gameWidth) - 2) / 2;   // -2 for borders

    // Ensure we don't go negative
    if (startY < 0)
        startY = 0;
    if (startX < 0)
        startX = 0;

    bool showBorders = _menuSystem && _menuSystem->getSettings().showBorders;
    int tileOffsetX = startX + (showBorders ? 1 : 0);
    int tileOffsetY = startY + (showBorders ? 1 : 0);

    // Draw game border (optional)
    if (showBorders) {
        attron(COLOR_PAIR(COLOR_BORDER));
        // Top border
        mvhline(startY, startX, ACS_HLINE, static_cast<int>(gameWidth) + 2);
        mvaddch(startY, startX, ACS_ULCORNER);
        mvaddch(startY, startX + static_cast<int>(gameWidth) + 1, ACS_URCORNER);
    }

    // Side borders and game area
    for (size_t y = 0; y < gameHeight; ++y) {
        if (showBorders) {
            mvaddch(startY + 1 + static_cast<int>(y), startX, ACS_VLINE);
            mvaddch(startY + 1 + static_cast<int>(y), startX + static_cast<int>(gameWidth) + 1, ACS_VLINE);
        }

        // Draw game tiles
        for (size_t x = 0; x < gameWidth; ++x) {
            char ch = getCharFromGameTile(static_cast<int>(x), static_cast<int>(y), game);
            int colorPair = getColorFromGameTile(static_cast<int>(x), static_cast<int>(y), game);

            attron(COLOR_PAIR(colorPair));
            mvaddch(tileOffsetY + static_cast<int>(y), tileOffsetX + static_cast<int>(x), ch);
            attroff(COLOR_PAIR(colorPair));
        }
    }

    // Bottom border
    if (showBorders) {
        attron(COLOR_PAIR(COLOR_BORDER));
        mvhline(startY + static_cast<int>(gameHeight) + 1, startX, ACS_HLINE, static_cast<int>(gameWidth) + 2);
        mvaddch(startY + static_cast<int>(gameHeight) + 1, startX, ACS_LLCORNER);
        mvaddch(startY + static_cast<int>(gameHeight) + 1, startX + static_cast<int>(gameWidth) + 1, ACS_LRCORNER);
        attroff(COLOR_PAIR(COLOR_BORDER));
    }

    // Draw game info
    drawInfo(game);

    // No switch message displayed

    // Refresh screen
    refresh();
}

GameKey NCursesGraphics::getInput() {
    if (!_initialized) {
        return GameKey::NONE;
    }

    int ch = getch();

    // Handle graphics switching first (works in any state)
    switch (ch) {
    case '1':
        return GameKey::KEY_1;
    case '2':
        return GameKey::KEY_2;
    case '3':
        return GameKey::KEY_3;
    case '4':
        return GameKey::KEY_4;
    case ERR:
        return GameKey::NONE;
    }

    // Handle escape sequences manually if NCurses arrow keys don't work
    if (ch == '[') {
        // This might be part of an arrow key sequence like [A, [B, [C, [D
        // Get the next character
        int next_ch = getch();
        if (next_ch != ERR) {

            // Handle menu navigation if in menu mode
            if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
                switch (next_ch) {
                case 'A': // Up arrow
                    _menuSystem->navigateUp();
                    return GameKey::NONE;
                case 'B': // Down arrow
                    _menuSystem->navigateDown();
                    return GameKey::NONE;
                default:
                    return GameKey::NONE;
                }
            }

            // Handle game input
            switch (next_ch) {
            case 'A': // Up arrow
                return GameKey::UP;
            case 'B': // Down arrow
                return GameKey::DOWN;
            case 'C': // Right arrow
                return GameKey::RIGHT;
            case 'D': // Left arrow
                return GameKey::LEFT;
            default:
                return GameKey::NONE;
            }
        }
    }

    // Handle menu navigation if in menu mode
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        switch (ch) {
        case KEY_UP:
            _menuSystem->navigateUp();
            return GameKey::NONE;
        case KEY_DOWN:
            _menuSystem->navigateDown();
            return GameKey::NONE;
        case 10:  // ENTER key
        case 13:  // ENTER key (alternative)
        case ' ': // SPACE key
            _menuSystem->selectCurrentItem();
            return GameKey::NONE;
        case 27: // ESC key
            _menuSystem->goBack();
            return GameKey::NONE;
        default:
            return GameKey::NONE;
        }
    }

    // Handle game input
    switch (ch) {
    case ERR: // No input available
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
    case '4':
        return GameKey::KEY_4;
    case 27: // ESC key
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
    // Reset existing color pairs based on current palette selection
    if (_altColorsActive) {
        // init_pair(COLOR_SNAKE_HEAD, COLOR_CYAN, COLOR_BLACK);
        // init_pair(COLOR_SNAKE_BODY, COLOR_BLUE, COLOR_BLACK);
        // init_pair(COLOR_FOOD, COLOR_YELLOW, COLOR_BLACK);
        // init_pair(COLOR_WALL, COLOR_YELLOW, COLOR_BLACK);
        // init_pair(COLOR_ICE, COLOR_BLACK, COLOR_BLACK);
        // init_pair(COLOR_BORDER, COLOR_YELLOW, COLOR_BLACK);
        // init_pair(COLOR_INFO, COLOR_WHITE, COLOR_BLACK);

    //change colors to have another color pair
    init_pair(COLOR_SNAKE_HEAD, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_SNAKE_BODY, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_FOOD, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_WALL, COLOR_WHITE, COLOR_WHITE);
    init_pair(COLOR_ICE, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_BORDER, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_INFO, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_FIRE_FOOD, COLOR_RED, COLOR_YELLOW);
    init_pair(COLOR_FROSTY_FOOD, COLOR_CYAN, COLOR_WHITE);
    init_pair(COLOR_FIRE_TILE, COLOR_RED, COLOR_BLACK);
    } else {
        init_pair(COLOR_SNAKE_HEAD, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_SNAKE_BODY, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_FOOD, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_WALL, COLOR_WHITE, COLOR_WHITE);
    init_pair(COLOR_ICE, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_BORDER, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_INFO, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_FIRE_FOOD, COLOR_RED, COLOR_YELLOW);
    init_pair(COLOR_FROSTY_FOOD, COLOR_CYAN, COLOR_WHITE);
    init_pair(COLOR_FIRE_TILE, COLOR_RED, COLOR_BLACK);
    }
}

void NCursesGraphics::drawInfo(const game_data& game) {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw info at the bottom
    attron(COLOR_PAIR(COLOR_INFO));

    // Snake length
    mvprintw(termHeight - 4, 2, "Snake Length: %d", game.get_snake_length(0));

    // FPS display (toggleable)
    if (_menuSystem && _menuSystem->getSettings().showFPS) {
        mvprintw(termHeight - 4, 28, "| FPS: %d", _frameRate);
    }

    attroff(COLOR_PAIR(COLOR_INFO));
}

char NCursesGraphics::getCharFromGameTile(int x, int y, const game_data& game) {
    // Check layer 2 first (snake and food)
    int layer2Value = game.get_map_value(x, y, 2);
    if (layer2Value == FOOD) {
        return '*';  // Food
    } else if (layer2Value == FIRE_FOOD) {
        return 'F';  // Fire food
    } else if (layer2Value == FROSTY_FOOD) {
        return 'I';  // Frosty food
    } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
        return '@'; // Snake head
    } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
        return 'o'; // Snake body
    }

    // Check layer 0 (terrain)
    int layer0Value = game.get_map_value(x, y, 0);
    if (layer0Value == GAME_TILE_WALL) {
        return '#'; // Wall
    } else if (layer0Value == GAME_TILE_ICE) {
        return '~';  // Ice
    } else if (layer0Value == GAME_TILE_FIRE) {
        return '^';  // Fire tile
    }

    return ' '; // Empty space
}

int NCursesGraphics::getColorFromGameTile(int x, int y, const game_data& game) {
    // Check layer 2 first (snake and food)
    int layer2Value = game.get_map_value(x, y, 2);
    if (layer2Value == FOOD) {
        return COLOR_FOOD;
    } else if (layer2Value == FIRE_FOOD) {
        return COLOR_FIRE_FOOD;
    } else if (layer2Value == FROSTY_FOOD) {
        return COLOR_FROSTY_FOOD;
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
    } else if (layer0Value == GAME_TILE_FIRE) {
        return COLOR_FIRE_TILE;
    }

    return 0; // Default color
}

void NCursesGraphics::setError(const std::string& error) {
    _errorMessage = error;
}

void NCursesGraphics::clearError() {
    _errorMessage.clear();
}

void NCursesGraphics::setSwitchMessage(const std::string& message, int timer) {
    _switchMessage = message;
    _switchMessageTimer = timer;
}

void NCursesGraphics::forceInputReadiness() {
    if (!_initialized) {
        return;
    }

    // Force multiple refreshes to ensure terminal is ready
    for (int i = 0; i < 3; i++) {
        flushinp(); // Clear input buffer
        refresh();  // Refresh display
        doupdate(); // Force update

        // Small delay between refreshes
        napms(10); // 10ms delay
    }

    // Final input buffer clear
    flushinp();
}

// Menu rendering methods
void NCursesGraphics::renderMenu(const game_data& game) {
    if (!_menuSystem) return;

    switch (_menuSystem->getCurrentState()) {
        case MenuState::MAIN_MENU:
            renderMainMenu();
            break;
        case MenuState::SETTINGS_MENU:
            renderSettingsMenu();
            break;
        case MenuState::CREDITS_PAGE:
            renderCreditsPage();
            break;
        case MenuState::INSTRUCTIONS_PAGE:
            renderInstructionsPage();
            break;
        case MenuState::ACHIEVEMENTS_PAGE:
            renderAchievementsPage(game);
            break;
        case MenuState::GAME_OVER:
            renderGameOverScreen();
            break;
        default:
            break;
    }
}

void NCursesGraphics::renderMainMenu() {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw title
    attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    drawCenteredText(termHeight / 4, _menuSystem->getCurrentTitle());
    attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int startY = termHeight / 2 - static_cast<int>(items.size()) / 2;
    drawMenuItems(items, _menuSystem->getCurrentSelection(), startY);

    // Draw footer
    attron(COLOR_PAIR(COLOR_INFO));
    drawCenteredText(termHeight - 3, "Use Arrow Keys to navigate, ENTER to select");
    drawCenteredText(termHeight - 2, "Press 1/2/3/4 to switch graphics libraries");
    attroff(COLOR_PAIR(COLOR_INFO));
}

void NCursesGraphics::renderSettingsMenu() {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw title
    attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    drawCenteredText(3, _menuSystem->getCurrentTitle());
    attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int startY = 6;
    drawMenuItems(items, _menuSystem->getCurrentSelection(), startY);

    // Draw footer
    attron(COLOR_PAIR(COLOR_INFO));
    drawCenteredText(termHeight - 4, "Use Arrow Keys to navigate, ENTER to toggle/adjust");
    drawCenteredText(termHeight - 3, "ESC to go back");
    attroff(COLOR_PAIR(COLOR_INFO));
}

void NCursesGraphics::drawCenteredText(int y, const std::string& text, int colorPair) {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    if (y < 0 || y >= termHeight)
        return;

    int x = (termWidth - static_cast<int>(text.length())) / 2;
    if (x < 0)
        x = 0;

    if (colorPair > 0)
        attron(COLOR_PAIR(colorPair));
    mvprintw(y, x, "%s", text.c_str());
    if (colorPair > 0)
        attroff(COLOR_PAIR(colorPair));
}

void NCursesGraphics::drawMenuItems(const std::vector<MenuItem>& items, int selection, int startY) {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    for (size_t i = 0; i < items.size(); ++i) {
        int y = startY + static_cast<int>(i);
        if (y >= termHeight - 1)
            break;

        const MenuItem& item = items[i];
        if (!item.selectable && item.text.empty())
            continue; // Skip spacers

        std::string displayText = item.text;
        if (static_cast<int>(i) == selection && item.selectable) {
            displayText = "> " + displayText + " <";
            attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
        } else if (item.selectable) {
            displayText = "  " + displayText;
            attron(COLOR_PAIR(COLOR_INFO));
        } else {
            attron(COLOR_PAIR(COLOR_BORDER));
        }

        drawCenteredText(y, displayText);
        attroff(A_BOLD | COLOR_PAIR(COLOR_SNAKE_HEAD) | COLOR_PAIR(COLOR_INFO) | COLOR_PAIR(COLOR_BORDER));
    }
}

void NCursesGraphics::renderCreditsPage() {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw title
    attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    drawCenteredText(2, _menuSystem->getCurrentTitle());
    attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

    // Draw content
    auto content = _menuSystem->getCreditsContent();
    int startY = 4;

    for (size_t i = 0; i < content.size() && startY + static_cast<int>(i) < termHeight - 2; ++i) {
        const std::string& line = content[i];
        if (line.empty())
            continue;

        int colorPair = COLOR_INFO;
        if (line.find("NIBBLER") != std::string::npos || line.find("LIBRARIES") != std::string::npos ||
            line.find("DESIGN") != std::string::npos || line.find("BONUS") != std::string::npos) {
            colorPair = COLOR_SNAKE_HEAD;
        } else if (line.find("•") != std::string::npos) {
            colorPair = COLOR_FOOD;
        }

        drawCenteredText(startY + static_cast<int>(i), line, colorPair);
    }

    // Draw footer
    attron(COLOR_PAIR(COLOR_BORDER));
    drawCenteredText(termHeight - 2, "Press ESC or ENTER to return to main menu");
    attroff(COLOR_PAIR(COLOR_BORDER));
}

void NCursesGraphics::renderInstructionsPage() {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw title
    attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    drawCenteredText(2, _menuSystem->getCurrentTitle());
    attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

    // Draw content
    auto content = _menuSystem->getInstructionsContent();
    int startY = 4;

    for (size_t i = 0; i < content.size() && startY + static_cast<int>(i) < termHeight - 2; ++i) {
        const std::string& line = content[i];
        if (line.empty())
            continue;

        int colorPair = COLOR_INFO;
        if (line.find("HOW TO") != std::string::npos || line.find("MENU") != std::string::npos ||
            line.find("GAME") != std::string::npos || line.find("SINGLE") != std::string::npos ||
            line.find("MULTIPLAYER") != std::string::npos || line.find("GRAPHICS") != std::string::npos) {
            colorPair = COLOR_SNAKE_HEAD;
        } else if (line.find("•") != std::string::npos || line.find("Key") != std::string::npos) {
            colorPair = COLOR_FOOD;
        }

        drawCenteredText(startY + static_cast<int>(i), line, colorPair);
    }

    // Draw footer
    attron(COLOR_PAIR(COLOR_BORDER));
    drawCenteredText(termHeight - 2, "Press ESC or ENTER to return to main menu");
    attroff(COLOR_PAIR(COLOR_BORDER));
}

void NCursesGraphics::renderAchievementsPage(const game_data& game) {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw title
    attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    drawCenteredText(2, _menuSystem->getCurrentTitle());
    attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

    // Draw content
    auto content = _menuSystem->getAchievementsContent(game);
    int startY = 4;

    for (size_t i = 0; i < content.size() && startY + static_cast<int>(i) < termHeight - 2; ++i) {
        const std::string& line = content[i];
        if (line.empty()) continue;

        int colorPair = COLOR_INFO;
        if (line.find("Unlocked") != std::string::npos) {
            colorPair = COLOR_FOOD;
        }

        drawCenteredText(startY + static_cast<int>(i), line, colorPair);
    }

    // Draw footer
    attron(COLOR_PAIR(COLOR_BORDER));
    drawCenteredText(termHeight - 2, "Press ESC or ENTER to return to main menu");
    attroff(COLOR_PAIR(COLOR_BORDER));
}

void NCursesGraphics::renderGameOverScreen() {
    int termHeight, termWidth;
    getmaxyx(stdscr, termHeight, termWidth);

    // Draw title
    attron(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);
    drawCenteredText(2, _menuSystem->getCurrentTitle());
    attroff(COLOR_PAIR(COLOR_SNAKE_HEAD) | A_BOLD);

    // Draw game over message
    attron(COLOR_PAIR(COLOR_FOOD) | A_BOLD);
    drawCenteredText(4, "Your snake has collided!");
    attroff(COLOR_PAIR(COLOR_FOOD) | A_BOLD);

    // Draw final score
    attron(COLOR_PAIR(COLOR_SNAKE_BODY));
    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredText(6, scoreText);
    attroff(COLOR_PAIR(COLOR_SNAKE_BODY));

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int selection = _menuSystem->getCurrentSelection();
    drawMenuItems(items, selection, 9);

    // Draw instructions
    attron(COLOR_PAIR(COLOR_INFO));
    drawCenteredText(termHeight - 4, "Use Arrow Keys to navigate, ENTER to select");
    drawCenteredText(termHeight - 3, "Press ESC to return to main menu");
    drawCenteredText(termHeight - 2, "Press 1/2/3/4 to switch graphics libraries");
    attroff(COLOR_PAIR(COLOR_INFO));
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
