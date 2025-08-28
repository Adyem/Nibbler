#include "MenuSystem.hpp"
#include "game_data.hpp"
#include <algorithm>

MenuSystem::MenuSystem() : _currentState(MenuState::MAIN_MENU), _currentSelection(0), _gameOverScore(0) {
    initializeMenus();
}

void MenuSystem::setState(MenuState state) {
    // Only reset selection if we're actually changing to a different state
    if (_currentState != state) {
        _currentState = state;
        _currentSelection = 0;

        if (state == MenuState::SETTINGS_MENU) {
            updateSettingsMenu();
        }
    }
}

void MenuSystem::navigateUp() {
    if (_currentSelection > 0) {
        _currentSelection--;

        // Skip non-selectable items
        const auto& items = getCurrentMenuItems();
        while (_currentSelection > 0 && !items[_currentSelection].selectable) {
            _currentSelection--;
        }
    }
}

void MenuSystem::navigateDown() {
    const auto& items = getCurrentMenuItems();
    if (_currentSelection < static_cast<int>(items.size()) - 1) {
        _currentSelection++;

        // Skip non-selectable items
        while (_currentSelection < static_cast<int>(items.size()) - 1 && !items[_currentSelection].selectable) {
            _currentSelection++;
        }
    }
}

void MenuSystem::selectCurrentItem() {
    switch (_currentState) {
        case MenuState::MAIN_MENU:
            switch (_currentSelection) {
                case 0: // Start Game
                    setState(MenuState::IN_GAME);
                    break;
                case 1: // Settings
                    setState(MenuState::SETTINGS_MENU);
                    break;
                case 2: // Credits
                    setState(MenuState::CREDITS_PAGE);
                    break;
                case 3: // Instructions
                    setState(MenuState::INSTRUCTIONS_PAGE);
                    break;
                case 4: // Achievements
                    setState(MenuState::ACHIEVEMENTS_PAGE);
                    break;
                case 5: // Exit
                    setState(MenuState::EXIT_REQUESTED);
                    break;
            }
            break;

        case MenuState::SETTINGS_MENU:
            // Handle settings selection based on current item
            if (_currentSelection == 0) toggleGameMode();
            else if (_currentSelection == 1) adjustGameSpeed(10);
            else if (_currentSelection == 2) toggleWrapAround();
            // Board size (index 3) is non-selectable - skip
            else if (_currentSelection == 4) toggleAdditionalFoodItems();
            else if (_currentSelection == 5) toggleAlternativeColors();
            else if (_currentSelection == 6) toggleGrid();
            else if (_currentSelection == 7) toggleFPS();

            updateSettingsMenu();
            break;

        case MenuState::CREDITS_PAGE:
        case MenuState::INSTRUCTIONS_PAGE:
        case MenuState::ACHIEVEMENTS_PAGE:
            goBack();
            break;

        case MenuState::GAME_OVER:
            switch (_currentSelection) {
                case 0: // Play Again
                    setState(MenuState::IN_GAME);
                    break;
                case 1: // Main Menu
                    setState(MenuState::MAIN_MENU);
                    break;
                case 2: // Exit
                    setState(MenuState::EXIT_REQUESTED);
                    break;
            }
            break;

        default:
            break;
    }
}

void MenuSystem::goBack() {
    switch (_currentState) {
        case MenuState::SETTINGS_MENU:
        case MenuState::CREDITS_PAGE:
        case MenuState::INSTRUCTIONS_PAGE:
        case MenuState::ACHIEVEMENTS_PAGE:
            setState(MenuState::MAIN_MENU);
            break;
        case MenuState::IN_GAME:
            setState(MenuState::MAIN_MENU);
            break;
        case MenuState::GAME_OVER:
            setState(MenuState::EXIT_REQUESTED);
            break;
        default:
            break;
    }
}

const std::vector<MenuItem>& MenuSystem::getCurrentMenuItems() const {
    switch (_currentState) {
        case MenuState::MAIN_MENU:
            return _mainMenuItems;
        case MenuState::SETTINGS_MENU:
            return _settingsMenuItems;
        case MenuState::GAME_OVER:
            return _gameOverMenuItems;
        default:
            return _mainMenuItems;
    }
}

std::string MenuSystem::getCurrentTitle() const {
    switch (_currentState) {
        case MenuState::MAIN_MENU:
            return "NIBBLER - SNAKE GAME";
        case MenuState::SETTINGS_MENU:
            return "GAME SETTINGS";
        case MenuState::CREDITS_PAGE:
            return "CREDITS";
        case MenuState::INSTRUCTIONS_PAGE:
            return "INSTRUCTIONS";
        case MenuState::ACHIEVEMENTS_PAGE:
            return "ACHIEVEMENTS";
        case MenuState::IN_GAME:
            return "NIBBLER";
        case MenuState::GAME_OVER:
            return "GAME OVER";
        default:
            return "NIBBLER";
    }
}

void MenuSystem::initializeMenus() {
    // Main menu
    _mainMenuItems = {
        MenuItem("Start Game"),
        MenuItem("Settings"),
        MenuItem("Credits"),
        MenuItem("Instructions"),
        MenuItem("Achievements"),
        MenuItem("Exit")
    };

    // Game over menu
    _gameOverMenuItems = {
        MenuItem("Play Again"),
        MenuItem("Main Menu"),
        MenuItem("Exit")
    };

    updateSettingsMenu();
}

void MenuSystem::updateSettingsMenu() {
    _settingsMenuItems.clear();

    std::string gameModeText = std::string("Game Mode: ") +
        (_settings.gameMode == GameMode::SINGLE_PLAYER ? "Single Player" : "Multiplayer");
    _settingsMenuItems.emplace_back(gameModeText);

    _settingsMenuItems.emplace_back("Game Speed: " + std::to_string(_settings.gameSpeed) + " FPS");
    _settingsMenuItems.emplace_back("Wrap Around Edges: " + std::string(_settings.wrapAroundEdges ? "ON" : "OFF"));
    _settingsMenuItems.emplace_back("Board Size: " + std::to_string(_settings.boardWidth) + "x" + std::to_string(_settings.boardHeight) + " (from command line)", false);
    _settingsMenuItems.emplace_back("Additional Foods: " + std::string(_settings.additionalFoodItems ? "ON" : "OFF"));
    _settingsMenuItems.emplace_back("Alternative Colors: " + std::string(_settings.useAlternativeColors ? "ON" : "OFF"));
    _settingsMenuItems.emplace_back("Show Grid: " + std::string(_settings.showGrid ? "ON" : "OFF"));
    _settingsMenuItems.emplace_back("Show FPS: " + std::string(_settings.showFPS ? "ON" : "OFF"));
    // No ghost/placeholder entries at the end
}

// Settings modification methods
void MenuSystem::toggleGameMode() {
    _settings.gameMode = (_settings.gameMode == GameMode::SINGLE_PLAYER) ?
        GameMode::MULTIPLAYER : GameMode::SINGLE_PLAYER;
}

void MenuSystem::adjustGameSpeed(int delta) {
    _settings.gameSpeed = std::max(10, std::min(120, _settings.gameSpeed + delta));
}

void MenuSystem::toggleWrapAround() {
    _settings.wrapAroundEdges = !_settings.wrapAroundEdges;
}

void MenuSystem::toggleAdditionalFoodItems() {
    _settings.additionalFoodItems = !_settings.additionalFoodItems;
}

void MenuSystem::adjustBoardSize(int widthDelta, int heightDelta) {
    if (widthDelta != 0) {
        _settings.boardWidth = std::max(10, std::min(100, _settings.boardWidth + widthDelta));
    }
    if (heightDelta != 0) {
        _settings.boardHeight = std::max(10, std::min(100, _settings.boardHeight + heightDelta));
    }
}

void MenuSystem::toggleAlternativeColors() {
    _settings.useAlternativeColors = !_settings.useAlternativeColors;
}

void MenuSystem::toggleGrid() {
    _settings.showGrid = !_settings.showGrid;
}

void MenuSystem::toggleFPS() {
    _settings.showFPS = !_settings.showFPS;
}

std::vector<std::string> MenuSystem::getCreditsContent() const {
    return {
        "NIBBLER - SNAKE GAME WITH DYNAMIC LIBRARIES",
        "",
        "Developed by: rperez-t and bvangene",
        "C++ Version: C++17",
        "",
        "LIBRARIES USED:",
        " NCurses - Enhanced terminal graphics with colors",
        " SDL2 - Modern windowed graphics with hardware acceleration",
        " OpenGL - Cross-platform graphics API",
        " Raylib - Simple and easy-to-use library for 2D games",
        "",
        "DESIGN PHILOSOPHY:",
        "This project demonstrates the power of dynamic library loading",
        "in C++. The game engine is completely separated from the",
        "graphics rendering, allowing seamless switching between",
        "different graphics libraries at runtime using dlopen/dlsym.",
        "",
        "The architecture follows strict separation of concerns:",
        "Game logic in main executable",
        "Graphics/input handling in dynamic libraries",
        "Clean interfaces for extensibility",
        "Menu system independent from graphics implementation",
        "",
        "BONUS FEATURES:",
        "Comprehensive menu system across all libraries",
        "Different gamemodes load a map in from the argument",
        "Customizable game settings",
        "4th library used",
        "Achievements tracking",
        "",
        // "Press ESC or ENTER to return to main menu"
    };
}

std::vector<std::string> MenuSystem::getInstructionsContent() const {
    return {
        "HOW TO PLAY NIBBLER",
        "",
        "MENU NAVIGATION:",
        " Arrow Keys - Navigate menu items",
        " ENTER/SPACE - Select menu item",
        " ESC - Go back to previous menu",
        " Keys 1,2,3 - Switch graphics libraries anytime",
        "",
        "GAME OBJECTIVE:",
        " Control your snake to eat food and grow longer.",
        " Avoid hitting walls or your own body!",
        " In the extra gamemodes use the special tiles for effects",
        "",
        "SINGLE PLAYER MODE:",
        " Arrow Keys - Control snake direction",
        " Eat the red food (*) to grow longer",
        " Game ends when snake hits wall or itself",
        "",
        "GRAPHICS LIBRARIES:",
        "Key 1 - NCurses (colored terminal graphics)",
        "Key 2 - SDL2 (modern windowed graphics)",
        "Key 3 - OpenGL (cross-platform graphics API)",
        "Key 4 - Raylib (simple 2D game library)",
        "",
        // "Press ESC or ENTER to return to main menu"
    };
}

std::vector<std::string> MenuSystem::getAchievementsContent(const game_data& game) const {
    std::vector<std::string> content = {
        "PLAYER ACHIEVEMENTS",
        ""
    };

    content.push_back(std::string("Reach length 50: ") + (game.get_achievement_snake50() ? "Unlocked" : "Locked"));
    content.push_back(std::string("Apples eaten: ") + std::to_string(game.get_apples_eaten()));
    content.push_back(std::string("Normal: ") + std::to_string(game.get_apples_normal_eaten()));
    content.push_back(std::string("Frosty: ") + std::to_string(game.get_apples_frosty_eaten()));
    content.push_back(std::string("Fire: ") + std::to_string(game.get_apples_fire_eaten()));
    content.push_back(std::string("Tiles stepped on:"));
    content.push_back(std::string("Normal: ") + std::to_string(game.get_tile_normal_steps()));
    content.push_back(std::string("Frosty: ") + std::to_string(game.get_tile_frosty_steps()));
    content.push_back(std::string("Fire: ") + std::to_string(game.get_tile_fire_steps()));
    content.push_back("");
    // content.push_back("Press ESC or ENTER to return to main menu");
    return content;
}

void MenuSystem::setGameOverScore(int score) {
    _gameOverScore = score;
}

int MenuSystem::getGameOverScore() const {
    return _gameOverScore;
}
