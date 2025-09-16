#include "GameEngine.hpp"
#include "console_utils.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "file_utils.hpp" // for game_rules & rule loading

static const char* slotName(int slot) {
    switch (slot) {
    case 0: return "NCurses";
    case 1: return "SDL2";
    case 2: return "OpenGL";
    case 3: return "Raylib";
    default: return "Unknown";
    }
}

GameEngine::GameEngine(int width, int height)
    : _gameData(width, height), _initialized(false), _gameStarted(false), _usingBonusMap(false) {
    clearError();

    // Init library key mapping to missing
    for (int i = 0; i < 4; ++i) { _libKeyMap[i] = -1; _libSlotAvailable[i] = false; }
    _defaultLibIndex = -1;

    // Initialize menu system with the actual board dimensions from command line
    GameSettings settings = _menuSystem.getSettings();
    settings.boardWidth = width;
    settings.boardHeight = height;
    _menuSystem.updateSettings(settings);

    // Final board size may change if a bonus map is loaded via -b
    std::cout << "Nibbler engine created." << std::endl;
}

GameEngine::~GameEngine() {
    // Cleanup is handled by destructors
}

int GameEngine::loadBonusMap(const char* path) {
    if (!path || !*path) {
        setError("Invalid bonus map path");
        return 1;
    }
    _gameData.set_map_name(path);
    
    // Read and validate rules from file first
    game_rules rules;
    if (read_game_rules(_gameData, rules) < 0 || rules.error) {
        setError(std::string("Failed to load bonus map: ") + path);
        return 1;
    }
    // Apply rules to game data (sizes, wrap, additional items, tiles, snake)
    if (load_rules_into_game_data(_gameData) < 0) {
        setError(std::string("Failed to apply bonus map rules: ") + path);
        return 1;
    }
    
    // Sync menu settings to reflect the loaded map so the UI shows correct state
    GameSettings settings = _menuSystem.getSettings();
    settings.wrapAroundEdges = (rules.wrap_around_edges != 0);
    settings.additionalFoodItems = (rules.additional_fruits != 0);
    if (!rules.custom_map.empty()) {
        settings.boardWidth = static_cast<int>(rules.custom_map[0].size());
        settings.boardHeight = static_cast<int>(rules.custom_map.size());
    }
    _menuSystem.updateSettings(settings);

    // Mark that we are using a bonus map so we don't wipe it on start
    _usingBonusMap = true;
    std::cout << "Loaded bonus map: " << path << std::endl;
    return 0;
}

int GameEngine::initialize(int preferredLibraryIndex) {
    if (_initialized) {
        return 0;
    }

    // Check if game data was initialized properly
    if (_gameData.get_error() != 0) {
        setError("Failed to initialize game data");
        return 1;
    }

    // Load graphics libraries
    if (loadDefaultLibraries() != 0) {
        setError("Failed to load graphics libraries");
        return 1;
    }

    // Switch to preferred slot via mapping (0..3). Fallback to default if missing.
    int targetSlot = preferredLibraryIndex;
    if (targetSlot < 0 || targetSlot > 3) targetSlot = 0;
    int actualIndex = (targetSlot >= 0 && targetSlot <= 3) ? _libKeyMap[targetSlot] : -1;
    if (actualIndex < 0) actualIndex = _defaultLibIndex;

    if (_libraryManager.switchToLibrary(actualIndex) != 0) {
        print_warning("Could not switch to preferred library, using default");
    } else if (!_libSlotAvailable[targetSlot]) {
        // Warn user that selected slot was unavailable and remapped
        const char* fallbackName = _libraryManager.getLibraryName(actualIndex);
        std::string msg = std::string("Requested ") + slotName(targetSlot) +
                          " not available; using " + (fallbackName ? fallbackName : "Unknown") + "";
        print_warning(msg);
    }

    // Initialize the current graphics library
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    if (!currentLib) {
        setError("No graphics library available");
        return 1;
    }

    if (currentLib->initialize() != 0) {
        setError(std::string("Failed to initialize graphics library: ") +
                 (currentLib->getError() ? currentLib->getError() : "Unknown error"));
        return 1;
    }

    // Set frame rate to 60 FPS
    currentLib->setFrameRate(60);

    _initialized = true;
    return 0;
}

void GameEngine::run() {
    if (!_initialized) {
        print_error("Error: Game engine not initialized");
        return;
    }

    // Set up menu system for current graphics library
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    if (currentLib) {
        currentLib->setMenuSystem(&_menuSystem);
    }

    std::cout << "Nibbler started with " << _libraryManager.getLibraryName(_libraryManager.getCurrentLibraryIndex()) << std::endl;
    std::cout << "Navigate menus with arrow keys, ENTER to select, ESC to go back" << std::endl;
    std::cout << "Press 1/2/3/4 to switch graphics libraries anytime" << std::endl;

    // Start the game loop
    gameLoop();

    // Cleanup
    // Re-acquire the current library in case it changed during the game
    currentLib = _libraryManager.getCurrentLibrary();
    if (currentLib) {
        currentLib->shutdown();
    }
}

const char* GameEngine::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

bool GameEngine::isInitialized() const {
    return _initialized;
}

void GameEngine::gameLoop() {
    bool shouldQuit = false;
    const int targetFPS = 60;
    const auto frameDuration = std::chrono::microseconds(1000000 / targetFPS);
    auto lastFrameTime = std::chrono::steady_clock::now();
    while (!shouldQuit) {
        auto frameStart = std::chrono::steady_clock::now();
        double deltaTime = std::chrono::duration<double>(frameStart - lastFrameTime).count();
        lastFrameTime = frameStart;

        IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
        if (!currentLib) {

            break;
        }

        // Handle input
        GameKey key = GameKey::NONE;
        try {
            key = currentLib->getInput();
        } catch (...) {
            print_error("Error: Graphics library crashed during input handling");
            shouldQuit = true;
            break;
        }

        // Only process input if we actually got a key
        bool inputReceived = (key != GameKey::NONE);
        if (inputReceived) {
            handleInput(key, shouldQuit);
        }

        // IMPORTANT: The input handler may have switched libraries.
        // Refresh the currentLib pointer to avoid using a stale (possibly destroyed) instance.
        currentLib = _libraryManager.getCurrentLibrary();
        if (!currentLib) {
            shouldQuit = true;
            break;
        }

        // Check if we should quit based on menu state
        if (_menuSystem.getCurrentState() == MenuState::EXIT_REQUESTED) {
            shouldQuit = true;
        }

        // Check if we just entered game mode and need to apply settings
        static MenuState lastState = MenuState::MAIN_MENU;
        if (_menuSystem.getCurrentState() == MenuState::IN_GAME && lastState != MenuState::IN_GAME) {
            applyMenuSettings();
        }
        lastState = _menuSystem.getCurrentState();

        // Update game logic only if we're in game mode
        bool gameUpdated = false;
        if (_menuSystem.getCurrentState() == MenuState::IN_GAME) {
            updateGame(shouldQuit, deltaTime);
            gameUpdated = true;
        }

        // Always render for better responsiveness, especially in menus
        // Only skip rendering if we're in game mode and nothing changed
        static MenuState previousState = MenuState::MAIN_MENU;
        bool stateChanged = (_menuSystem.getCurrentState() != previousState);
        previousState = _menuSystem.getCurrentState();

        bool shouldRender = true;
        if (_menuSystem.getCurrentState() == MenuState::IN_GAME && !inputReceived && !gameUpdated && !stateChanged) {
            shouldRender = false;
        }

        if (shouldRender) {
            // Render the game
            try {
                renderGame();
            } catch (...) {
                print_error("Error: Graphics library crashed during rendering");
                shouldQuit = true;
                break;
            }
        }

        // Re-check current library in case rendering triggered a change (defensive)
        currentLib = _libraryManager.getCurrentLibrary();
        if (!currentLib) {
            shouldQuit = true;
            break;
        }

        // Check if library wants to quit
        if (!currentLib->shouldContinue()) {

            shouldQuit = true;
        }

        // Check for graphics library errors
        const char* error = currentLib->getError();
        if (error) {
            print_error(std::string("Graphics library error: ") + error);
            shouldQuit = true;
        }

        // Frame rate limiting for smooth 60 FPS
        auto frameEnd = std::chrono::steady_clock::now();
        auto elapsed = frameEnd - frameStart;
        if (elapsed < frameDuration) {
            std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }
}

void GameEngine::handleInput(GameKey key, bool& shouldQuit) {
    // Handle graphics library switching (works in any state)
    switch (key) {
    case GameKey::KEY_1:
        switchGraphicsLibrary(0);
        return;
    case GameKey::KEY_2:
        switchGraphicsLibrary(1);
        return;
    case GameKey::KEY_3:
        switchGraphicsLibrary(2);
        return;
    case GameKey::KEY_4:
        switchGraphicsLibrary(3);
        return;
    default:
        break;
    }

    // Handle menu state transitions
    if (_menuSystem.getCurrentState() == MenuState::IN_GAME) {
        // Handle game input
        switch (key) {
        case GameKey::UP:
            _gameData.set_direction_moving(0, DIRECTION_UP);
            if (!_gameStarted) {
                _gameStarted = true;
                std::cout << "Game Started! Use arrow keys to control the snake." << std::endl;
            }
            break;
        case GameKey::DOWN:
            _gameData.set_direction_moving(0, DIRECTION_DOWN);
            if (!_gameStarted) {
                _gameStarted = true;
                std::cout << "Game Started! Use arrow keys to control the snake." << std::endl;
            }
            break;
        case GameKey::LEFT:
            _gameData.set_direction_moving(0, DIRECTION_LEFT);
            if (!_gameStarted) {
                _gameStarted = true;
                std::cout << "Game Started! Use arrow keys to control the snake." << std::endl;
            }
            break;
        case GameKey::RIGHT:
            _gameData.set_direction_moving(0, DIRECTION_RIGHT);
            if (!_gameStarted) {
                _gameStarted = true;
                std::cout << "Game Started! Use arrow keys to control the snake." << std::endl;
            }
            break;
        case GameKey::ESCAPE:
        case GameKey::QUIT:
            // Go back to main menu instead of quitting directly
            _menuSystem.setState(MenuState::MAIN_MENU);
            break;
        case GameKey::NONE:
        default:
            // No input or unknown input, do nothing
            break;
        }
    } else {
        // In menu mode - input is handled by the graphics library
        // Only exit on ESC from the main menu
        if (key == GameKey::ESCAPE && _menuSystem.getCurrentState() == MenuState::MAIN_MENU) {
            shouldQuit = true;
        }
    }
}

void GameEngine::updateGame(bool& /* shouldQuit */, double deltaTime) {
    // Only update game logic if the game has started
    if (!_gameStarted) {
        return;
    }

    // Update game logic based on elapsed time
    int updateResult = _gameData.update_game_map(deltaTime);
    if (updateResult != 0) {
        int finalScore = _gameData.get_snake_length(0);
        std::cout << "Game Over! Snake collided. Final length: " << finalScore << std::endl;
        std::cout << "Showing game over screen..." << std::endl;

        // Set the final score and go to game over screen
        _menuSystem.setGameOverScore(finalScore);
        _menuSystem.setState(MenuState::GAME_OVER);

        // Reset game state for next game
        _gameStarted = false;
        _gameData.reset_board();
    }
}

void GameEngine::renderGame() {
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    if (currentLib) {
        currentLib->render(_gameData);
    }
}

int GameEngine::loadDefaultLibraries() {
    // Table of intended slots and their library paths
    struct Entry { int slot; const char* path; } entries[] = {
        {0, "./dllibs/lib_ncurses.so"},
        {1, "./dllibs/lib_sdl2.so"},
        {2, "./dllibs/lib_opengl.so"},
        {3, "./dllibs/lib_raylib.so"}
    };

    for (const auto& e : entries) {
        if (_libraryManager.loadLibrary(e.path) != 0) {
            // Mark missing and report error for this specific library
            _libKeyMap[e.slot] = -1;
            std::string err = std::string("Failed to load ") + slotName(e.slot) + ": " + _libraryManager.getError();
            print_error(err);
        } else {
            // New library appended at the end
            size_t countAfter = _libraryManager.getLibraryCount();
            int newIndex = static_cast<int>(countAfter - 1);
            _libKeyMap[e.slot] = newIndex;
            _libSlotAvailable[e.slot] = true;
            if (_defaultLibIndex < 0) {
                _defaultLibIndex = newIndex;
            }
        }
    }

    if (_libraryManager.getLibraryCount() == 0) {
        setError("No graphics libraries could be loaded");
        return 1;
    }

    // For each missing slot, map to default and warn user
    for (int slot = 0; slot < 4; ++slot) {
        if (_libKeyMap[slot] < 0) {
            _libKeyMap[slot] = _defaultLibIndex;
            const char* fallbackName = _libraryManager.getLibraryName(_defaultLibIndex);
            std::string warn = std::string("Library '") + slotName(slot) +
                               "' unavailable; mapping key " + std::to_string(slot + 1) +
                               " to '" + (fallbackName ? fallbackName : "Unknown") + "'";
            print_warning(warn);
        }
    }

    return 0;
}

void GameEngine::switchGraphicsLibrary(int librarySlot) {
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();

    // Translate desired slot (0..3) to actual loaded library index
    int actualIndex = (librarySlot >= 0 && librarySlot < 4) ? _libKeyMap[librarySlot] : -1;
    if (actualIndex < 0) actualIndex = _defaultLibIndex;
    bool remapped = (librarySlot < 0 || librarySlot >= 4) ? false : !_libSlotAvailable[librarySlot];

    // Don't switch if we're already using this library
    if (actualIndex == _libraryManager.getCurrentLibraryIndex()) {
        const char* currentLibName = _libraryManager.getLibraryName(actualIndex);
        std::string message = std::string("Already using ") + (currentLibName ? currentLibName : "Unknown Library");
        if (currentLib) {
            currentLib->setSwitchMessage(message, 120); // Show for 2 seconds at 60 FPS
        }
        return;
    }

    // Get current and target library names for logging
    const char* currentLibName = _libraryManager.getLibraryName(_libraryManager.getCurrentLibraryIndex());
    const char* targetLibName = _libraryManager.getLibraryName(actualIndex);

    std::cout << "Switching graphics library from "
              << (currentLibName ? currentLibName : "none")
              << " to " << (targetLibName ? targetLibName : "unknown") << std::endl;
    if (remapped && currentLib) {
        std::string message = std::string("Requested ") + slotName(librarySlot) +
                              " not available; using " + (targetLibName ? targetLibName : "Unknown") + "";
        currentLib->setSwitchMessage(message, 180);
    }

    // Shutdown current library
    if (currentLib) {
        currentLib->shutdown();

        // Small delay to ensure clean shutdown
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::this_thread::yield();
    }

    // Switch to new library
    if (_libraryManager.switchToLibrary(actualIndex) == 0) {
        IGraphicsLibrary* newLib = _libraryManager.getCurrentLibrary();
        if (newLib) {
            // Try to initialize the new library
            int initResult = newLib->initialize();

            if (initResult == 0) {
                newLib->setFrameRate(60);

                // Set up menu system for the new library
                newLib->setMenuSystem(&_menuSystem);

                // Special handling for different libraries
                const char* newLibName = _libraryManager.getLibraryName(actualIndex);
                if (newLibName && std::string(newLibName) == "NCurses") {
                    // Try to bring terminal to front on macOS
                    std::system("osascript -e 'tell application \"Terminal\" to activate' 2>/dev/null || true");

                    // Additional delay to ensure focus transfer
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    // Force NCurses to be ready for input
                    // This will be handled by the forceInputReadiness() method called in initialize()
                } else if (newLibName && (std::string(newLibName).find("Raylib") != std::string::npos ||
                                        std::string(newLibName).find("OpenGL") != std::string::npos)) {
                    // Small delay for OpenGL-based libraries to ensure context is ready
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }

                std::string message = std::string("Switched to: ") + (newLibName ? newLibName : "Unknown Library");
                newLib->setSwitchMessage(message, 120); // Show for 2 seconds at 60 FPS
            } else {
                std::string errorMsg = "Failed to initialize new graphics library: ";
                errorMsg += (newLib->getError() ? newLib->getError() : "Unknown error");

                // Switch back to the previous library
                if (currentLib && currentLib->initialize() == 0) {
                    currentLib->setFrameRate(60);
                    currentLib->setMenuSystem(&_menuSystem);
                    _libraryManager.switchToLibrary(0); // Assume NCurses is at index 0
                    currentLib->setSwitchMessage("Restored previous graphics library", 120);
                } else {
                    // If we can't restore, show error in the failed new library
                    newLib->setSwitchMessage("Failed to restore previous graphics library!", 180);
                }
            }
        } else {
            // Show error in current library if we can't get new library instance
            if (currentLib) {
                currentLib->setSwitchMessage("Failed to get new graphics library instance", 180);
            }
        }
    } else {
        // Show error in current library if switching failed
        if (currentLib) {
            std::string message = std::string("Failed to switch to library: ") + _libraryManager.getError();
            currentLib->setSwitchMessage(message, 180);
        }
    }
}

void GameEngine::setError(const std::string& error) {
    _errorMessage = error;
}

void GameEngine::clearError() {
    _errorMessage.clear();
}

void GameEngine::applyMenuSettings() {
    const GameSettings& settings = _menuSystem.getSettings();
    
    // If a bonus map is active, preserve its tiles and geometry.
    if (_usingBonusMap) {
        // Only apply dynamic settings that don't destroy the map
        IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
        if (currentLib) {
            currentLib->setFrameRate(settings.gameSpeed);
        }
        // Do NOT resize or reset the board; rules have already been applied
        _gameStarted = false;
        std::cout << "Using bonus map settings; board preserved." << std::endl;
        return;
    }

    // Apply board size for standard mode
    _gameData.resize_board(settings.boardWidth, settings.boardHeight);

    // Apply wrap around and additional items
    _gameData.set_wrap_around_edges(settings.wrapAroundEdges ? 1 : 0);
    _gameData.set_additional_food_items(settings.additionalFoodItems ? 1 : 0);

    // Apply frame rate to current graphics library
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    if (currentLib) {
        currentLib->setFrameRate(settings.gameSpeed);
    }

    // Reset game state for new game (standard mode only)
    _gameData.reset_board();
    _gameStarted = false;

    std::cout << "Game settings applied:" << std::endl;
    std::cout << "  Board size: " << settings.boardWidth << "x" << settings.boardHeight << std::endl;
    std::cout << "  Game mode: " << (settings.gameMode == GameMode::SINGLE_PLAYER ? "Single Player" : "Multiplayer") << std::endl;
    std::cout << "  Speed: " << settings.gameSpeed << " FPS" << std::endl;
    std::cout << "  Wrap around: " << (settings.wrapAroundEdges ? "ON" : "OFF") << std::endl;
}

void GameEngine::performIntermediateSwitch(int intermediateIndex, int finalIndex) {
    // First switch to the intermediate library (SDL2 to clean OpenGL state)
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();

    if (currentLib) {
        currentLib->shutdown();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // Switch to intermediate library
    if (_libraryManager.switchToLibrary(intermediateIndex) == 0) {
        IGraphicsLibrary* intermediateLib = _libraryManager.getCurrentLibrary();
        if (intermediateLib && intermediateLib->initialize() == 0) {
            intermediateLib->setFrameRate(60);
            intermediateLib->setMenuSystem(&_menuSystem);

            // Very brief operation with the intermediate library to clean OpenGL state
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            // Now shutdown the intermediate library
            intermediateLib->shutdown();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    // Now switch to the final library (Raylib)
    if (_libraryManager.switchToLibrary(finalIndex) == 0) {
        IGraphicsLibrary* finalLib = _libraryManager.getCurrentLibrary();
        if (finalLib && finalLib->initialize() == 0) {
            finalLib->setFrameRate(60);
            finalLib->setMenuSystem(&_menuSystem);

            const char* finalLibName = _libraryManager.getLibraryName(finalIndex);
            std::string message = std::string("Switched to: ") + (finalLibName ? finalLibName : "Unknown Library");
            finalLib->setSwitchMessage(message, 120);
        }
    }
}
