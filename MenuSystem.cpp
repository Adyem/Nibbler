#include "MenuSystem.hpp"
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
                case 4: // Exit
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
            else if (_currentSelection == 4) toggleAlternativeColors();
            else if (_currentSelection == 5) toggleGrid();
            else if (_currentSelection == 6) toggleFPS();
            else if (_currentSelection == 8) goBack(); // Back to Main Menu (adjusted index)

            updateSettingsMenu();
            break;
            
        case MenuState::CREDITS_PAGE:
        case MenuState::INSTRUCTIONS_PAGE:
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
            return "NIBBLER";
        case MenuState::SETTINGS_MENU:
            return "GAME SETTINGS";
        case MenuState::CREDITS_PAGE:
            return "CREDITS";
        case MenuState::INSTRUCTIONS_PAGE:
            return "INSTRUCTIONS";
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
    _settingsMenuItems.emplace_back("Alternative Colors: " + std::string(_settings.useAlternativeColors ? "ON" : "OFF"));
    _settingsMenuItems.emplace_back("Show Grid: " + std::string(_settings.showGrid ? "ON" : "OFF"));
    _settingsMenuItems.emplace_back("Show FPS: " + std::string(_settings.showFPS ? "ON" : "OFF"));
    
    _settingsMenuItems.emplace_back("", false); // Spacer
    _settingsMenuItems.emplace_back("Back to Main Menu");
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
        "Developed by: rperez-t and bvangene",
        "C++ Version: C++17",
        "",
        "LIBRARIES USED:",
        "NCurses - Enhanced terminal graphics with colors",
        "SDL2 - Modern windowed graphics with hardware acceleration",
        "SFML - Advanced graphics library (coming soon)",
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
        "",
        "BONUS FEATURES:",
        "Comprehensive menu system across all libraries",
        "Multiplayer support (two snakes, one food!)",
        "Customizable game settings graphicaly (skins, background, grid, etc.)"
    };
}

std::vector<std::string> MenuSystem::getInstructionsContent() const {
    return {
        "MENU NAVIGATION:",
        "Arrow Keys (UP/DOWN) - Navigate menu items",
        "ENTER/SPACE - Select menu item",
        "ESC - Go back to previous menu",
        "Keys 1,2,3 - Switch graphics libraries anytime",
        "",
        "GAME OBJECTIVE:",
        "Control your snake to eat food and grow longer.",
        "Avoid hitting walls or your own body!",
        "",
        "SINGLE PLAYER MODE:",
        "Arrow Keys - Control snake direction",
        "Eat the red food (*) to grow longer",
        "Game ends when snake hits wall or itself",
        "",
        "MULTIPLAYER MODE:",
        "Two snakes compete for the same food!",
        "- Player 1: Arrow Keys (UP/DOWN/LEFT/RIGHT)",
        "- Player 2: WASD Keys (W=up, S=down, A=left, D=right)",
        "- Both snakes grow when eating food",
        "- Game ends when any snake collides",
        "- Winner is the snake with highest score!",
        "",
        "GRAPHICS LIBRARIES:",
        "Key 1 - NCurses (colored terminal graphics)",
        "Key 2 - SDL2 (modern windowed graphics)",
        "Key 3 - SFML (advanced graphics with effects)",
        ""
    };
}

void MenuSystem::setGameOverScore(int score) {
    _gameOverScore = score;
}

int MenuSystem::getGameOverScore() const {
    return _gameOverScore;
}
