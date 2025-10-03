#pragma once

#include "game_data.hpp"
#include "LibraryManager.hpp"
#include "IGraphicsLibrary.hpp"
#include "MenuSystem.hpp"
#include <string>
#include <optional>

#include "file_utils.hpp"

class GameEngine {
  public:
    GameEngine(int width, int height);
    ~GameEngine();
    int initialize(int preferredLibraryIndex = 0);
    void run();
    const char* getError() const;
    bool isInitialized() const;
  // Bonus: load a custom game mode / map file (nibbler rules file)
  int loadBonusMap(const char* path);

  private:
    game_data _gameData;
    LibraryManager _libraryManager;
    MenuSystem _menuSystem;
    bool _initialized;
    std::string _errorMessage;
    bool _gameStarted;
    bool _usingBonusMap;
    int _libKeyMap[4];
    int _defaultLibIndex;
    bool _libSlotAvailable[4];
    int _lastAppliedFPS;

    void gameLoop();
    void handleInput(GameKey key, bool& shouldQuit);
    void updateGame(bool& shouldQuit, double deltaTime);
    void renderGame();
    void applyMenuSettings();
    void syncBonusSettings();
    int loadDefaultLibraries();
    void switchGraphicsLibrary(int libraryIndex);
    void performIntermediateSwitch(int intermediateIndex, int finalIndex);
    void handleGameOver();
    void setError(const std::string& error);
    void clearError();

    std::optional<game_rules> _cachedBonusRules;
};
