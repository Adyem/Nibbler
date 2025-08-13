#include "GameEngine.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>

GameEngine::GameEngine(int width, int height)
    : _gameData(width, height), _initialized(false), _gameStarted(false) {
    clearError();

    // Initialize menu system with the actual board dimensions from command line
    GameSettings settings = _menuSystem.getSettings();
    settings.boardWidth = width;
    settings.boardHeight = height;
    _menuSystem.updateSettings(settings);

    std::cout << "Nibbler initialized with board size: " << width << "x" << height << std::endl;
}

GameEngine::~GameEngine() {
    // Cleanup is handled by destructors
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

    // Try to switch to the preferred library
    if (preferredLibraryIndex >= 0 && preferredLibraryIndex < static_cast<int>(_libraryManager.getLibraryCount())) {
        if (_libraryManager.switchToLibrary(preferredLibraryIndex) != 0) {
            std::cerr << "Warning: Could not switch to preferred library, using default" << std::endl;
        }
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
        std::cerr << "Error: Game engine not initialized" << std::endl;
        return;
    }

    // Set up menu system for current graphics library
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    if (currentLib) {
        currentLib->setMenuSystem(&_menuSystem);
    }

    std::cout << "Nibbler started with " << _libraryManager.getLibraryName(_libraryManager.getCurrentLibraryIndex()) << std::endl;
    std::cout << "Navigate menus with arrow keys, ENTER to select, ESC to go back" << std::endl;
    std::cout << "Press 1/2/3 to switch graphics libraries anytime" << std::endl;

    // Start the game loop
    gameLoop();

    // Cleanup
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



    while (!shouldQuit) {
        auto frameStart = std::chrono::steady_clock::now();

        IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
        if (!currentLib) {

            break;
        }

        // Handle input
        GameKey key = GameKey::NONE;
        try {
            key = currentLib->getInput();
        } catch (...) {
            std::cerr << "Error: Graphics library crashed during input handling" << std::endl;
            shouldQuit = true;
            break;
        }

        // Only process input if we actually got a key
        bool inputReceived = (key != GameKey::NONE);
        if (inputReceived) {
            handleInput(key, shouldQuit);
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
            updateGame(shouldQuit);
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
                std::cerr << "Error: Graphics library crashed during rendering" << std::endl;
                shouldQuit = true;
                break;
            }
        }

        // Check if library wants to quit
        if (!currentLib->shouldContinue()) {

            shouldQuit = true;
        }

        // Check for graphics library errors
        const char* error = currentLib->getError();
        if (error) {
            std::cerr << "Graphics library error: " << error << std::endl;
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
        // We just need to handle ESC for quitting from main menu and game over screen
        if (key == GameKey::ESCAPE) {
            if (_menuSystem.getCurrentState() == MenuState::MAIN_MENU ||
                _menuSystem.getCurrentState() == MenuState::GAME_OVER) {
                shouldQuit = true;
            }
        }
    }
}

void GameEngine::updateGame(bool& /* shouldQuit */) {
    // Only update game logic if the game has started
    if (!_gameStarted) {
        return;
    }

    // Update game logic every frame - the game_data class handles its own timing
    int updateResult = _gameData.update_game_map();
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
    // Try to load the NCurses library first (index 0)
    if (_libraryManager.loadLibrary("./lib_ncurses.so") != 0) {
        std::cerr << "Warning: Failed to load NCurses library: " << _libraryManager.getError() << std::endl;
    }

    // Try to load the SDL2 library (index 1)
    if (_libraryManager.loadLibrary("./lib_sdl2.so") != 0) {
        std::cerr << "Warning: Failed to load SDL2 library: " << _libraryManager.getError() << std::endl;
    }

    // Try to load the SFML library (index 2)
    if (_libraryManager.loadLibrary("./lib_sfml.so") != 0) {
        std::cerr << "Warning: Failed to load SFML library: " << _libraryManager.getError() << std::endl;
    }

    if (_libraryManager.getLibraryCount() == 0) {
        setError("No graphics libraries could be loaded");
        return 1;
    }

    return 0;
}

void GameEngine::switchGraphicsLibrary(int libraryIndex) {
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();

    // Check if the library index is valid
    if (libraryIndex < 0 || libraryIndex >= static_cast<int>(_libraryManager.getLibraryCount())) {
        // Display message in the current graphics library
        std::string message = "Graphics library " + std::to_string(libraryIndex + 1) + " is not available";
        if (currentLib) {
            currentLib->setSwitchMessage(message, 120); // Show for 2 seconds at 60 FPS
        }
        return;
    }

    // Don't switch if we're already using this library
    if (libraryIndex == _libraryManager.getCurrentLibraryIndex()) {
        const char* currentLibName = _libraryManager.getLibraryName(libraryIndex);
        std::string message = std::string("Already using ") + (currentLibName ? currentLibName : "Unknown Library");
        if (currentLib) {
            currentLib->setSwitchMessage(message, 120); // Show for 2 seconds at 60 FPS
        }
        return;
    }

    // Shutdown current library
    if (currentLib) {
        currentLib->shutdown();

        // Small delay to ensure clean shutdown, especially for SDL2
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // Switch to new library
    if (_libraryManager.switchToLibrary(libraryIndex) == 0) {
        IGraphicsLibrary* newLib = _libraryManager.getCurrentLibrary();
        if (newLib) {
            // Try to initialize the new library
            int initResult = newLib->initialize();

            if (initResult == 0) {
                newLib->setFrameRate(60);

                // Set up menu system for the new library
                newLib->setMenuSystem(&_menuSystem);

                // Special handling for NCurses to ensure it gets focus
                const char* newLibName = _libraryManager.getLibraryName(libraryIndex);
                if (newLibName && std::string(newLibName) == "NCurses") {
                    // Try to bring terminal to front on macOS
                    std::system("osascript -e 'tell application \"Terminal\" to activate' 2>/dev/null || true");

                    // Additional delay to ensure focus transfer
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                    // Force NCurses to be ready for input (cast to NCursesGraphics if needed)
                    // This will be handled by the forceInputReadiness() method called in initialize()
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

    // Apply board size
    _gameData.resize_board(settings.boardWidth, settings.boardHeight);

    // Apply wrap around setting
    _gameData.set_wrap_around_edges(settings.wrapAroundEdges ? 1 : 0);

    // Apply frame rate to current graphics library
    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    if (currentLib) {
        currentLib->setFrameRate(settings.gameSpeed);
    }

    // Reset game state for new game
    _gameData.reset_board();
    _gameStarted = false;

    std::cout << "Game settings applied:" << std::endl;
    std::cout << "  Board size: " << settings.boardWidth << "x" << settings.boardHeight << std::endl;
    std::cout << "  Game mode: " << (settings.gameMode == GameMode::SINGLE_PLAYER ? "Single Player" : "Multiplayer") << std::endl;
    std::cout << "  Speed: " << settings.gameSpeed << " FPS" << std::endl;
    std::cout << "  Wrap around: " << (settings.wrapAroundEdges ? "ON" : "OFF") << std::endl;
}