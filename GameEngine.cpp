#include "GameEngine.hpp"
#include <iostream>
#include <chrono>
#include <thread>

GameEngine::GameEngine(int width, int height)
    : _gameData(width, height), _initialized(false), _gameStarted(false) {
    clearError();
}

GameEngine::~GameEngine() {
    // Cleanup is handled by destructors
}

int GameEngine::initialize() {
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

    // Initialize the first graphics library
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

    IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
    std::cout << "Nibbler started with " << _libraryManager.getLibraryName(0) << std::endl;
    std::cout << "Use arrow keys to move, 1/2/3 to switch graphics, ESC to quit" << std::endl;


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

        handleInput(key, shouldQuit);



        // Update game logic
        updateGame(shouldQuit);



        // Render the game
        try {
            renderGame();
        } catch (...) {
            std::cerr << "Error: Graphics library crashed during rendering" << std::endl;
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
        case GameKey::KEY_1:
            switchGraphicsLibrary(0);
            break;
        case GameKey::KEY_2:
            switchGraphicsLibrary(1);
            break;
        case GameKey::KEY_3:
            switchGraphicsLibrary(2);
            break;
        case GameKey::ESCAPE:
        case GameKey::QUIT:
            shouldQuit = true;
            break;
        case GameKey::NONE:
        default:
            // No input or unknown input, do nothing
            break;
    }
}

void GameEngine::updateGame(bool& shouldQuit) {
    // Only update game logic if the game has started
    if (!_gameStarted) {
        return;
    }



    // Update game logic every frame - the game_data class handles its own timing
    int updateResult = _gameData.update_game_map();
    if (updateResult != 0) {
        std::cout << "Game Over! Snake collided. Update result: " << updateResult << std::endl;
        std::cout << "Press any key to exit..." << std::endl;
        shouldQuit = true;
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
    if (libraryIndex >= 0 && libraryIndex < static_cast<int>(_libraryManager.getLibraryCount())) {
        // Shutdown current library
        IGraphicsLibrary* currentLib = _libraryManager.getCurrentLibrary();
        if (currentLib) {
            currentLib->shutdown();
        }

        // Switch to new library
        if (_libraryManager.switchToLibrary(libraryIndex) == 0) {
            IGraphicsLibrary* newLib = _libraryManager.getCurrentLibrary();
            if (newLib) {
                if (newLib->initialize() == 0) {
                    newLib->setFrameRate(60);
                    std::cout << "Switched to: " << _libraryManager.getLibraryName(libraryIndex) << std::endl;
                } else {
                    std::cerr << "Failed to initialize new graphics library" << std::endl;
                }
            }
        }
    }
}

void GameEngine::setError(const std::string& error) {
    _errorMessage = error;
}

void GameEngine::clearError() {
    _errorMessage.clear();
}