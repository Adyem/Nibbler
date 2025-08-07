#ifndef GAMEENGINE_HPP
#define GAMEENGINE_HPP

#include "game_data.hpp"
#include "LibraryManager.hpp"
#include "IGraphicsLibrary.hpp"
#include <string>

class GameEngine {
public:
    GameEngine(int width, int height);
    ~GameEngine();

    // Initialize the game engine
    int initialize();

    // Run the main game loop
    void run();

    // Get error message
    const char* getError() const;

    // Check if engine is properly initialized
    bool isInitialized() const;

private:
    game_data _gameData;
    LibraryManager _libraryManager;
    bool _initialized;
    std::string _errorMessage;

    // Game state
    bool _gameStarted;    // Whether the game has started (user pressed a key)

    // Game loop components
    void gameLoop();
    void handleInput(GameKey key, bool& shouldQuit);
    void updateGame(bool& shouldQuit);
    void renderGame();

    // Library management
    int loadDefaultLibraries();
    void switchGraphicsLibrary(int libraryIndex);

    // Helper methods
    void setError(const std::string& error);
    void clearError();
};

#endif // GAMEENGINE_HPP