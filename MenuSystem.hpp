#pragma once

#include <string>
#include <vector>

enum class MenuState { MAIN_MENU,
                       SETTINGS_MENU,
                       CREDITS_PAGE,
                       INSTRUCTIONS_PAGE,
                       IN_GAME,
                       GAME_OVER,
                       EXIT_REQUESTED };

enum class GameMode { SINGLE_PLAYER,
                      MULTIPLAYER };

struct GameSettings {
    GameMode gameMode = GameMode::SINGLE_PLAYER;
    int gameSpeed = 60;
    bool wrapAroundEdges = false;
    int boardWidth = 15;
    int boardHeight = 10;
    bool useAlternativeColors = false;
    bool showGrid = false;
    bool showFPS = true;
};

struct MenuItem {
    std::string text;
    bool selectable;
    MenuItem(const std::string& t, bool s = true) : text(t), selectable(s) {}
};

class MenuSystem {
  public:
    MenuSystem();
    virtual ~MenuSystem() = default;

    MenuState getCurrentState() const {
        return _currentState;
    }
    void setState(MenuState state);
    void navigateUp();
    void navigateDown();
    void selectCurrentItem();
    void goBack();

    const GameSettings& getSettings() const {
        return _settings;
    }
    void updateSettings(const GameSettings& settings) {
        _settings = settings;
    }

    const std::vector<MenuItem>& getCurrentMenuItems() const;
    int getCurrentSelection() const {
        return _currentSelection;
    }
    std::string getCurrentTitle() const;

    std::vector<std::string> getCreditsContent() const;
    std::vector<std::string> getInstructionsContent() const;
    std::vector<std::string> getSettingsContent() const;

    void toggleGameMode();
    void adjustGameSpeed(int delta);
    void toggleWrapAround();
    void adjustBoardSize(int widthDelta, int heightDelta);
    void toggleAlternativeColors();
    void toggleGrid();
    void toggleFPS();

    void setGameOverScore(int score);
    int getGameOverScore() const;

  private:
    MenuState _currentState;
    int _currentSelection;
    GameSettings _settings;
    int _gameOverScore;

    std::vector<MenuItem> _mainMenuItems;
    std::vector<MenuItem> _settingsMenuItems;
    std::vector<MenuItem> _gameOverMenuItems;

    void initializeMenus();
    void updateSettingsMenu();
    void clampSelection();
};
