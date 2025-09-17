#pragma once

#include <string>
#include <vector>

class game_data;

enum class MenuState {
    MAIN_MENU,
    SETTINGS_MENU,
    CREDITS_PAGE,
    INSTRUCTIONS_PAGE,
    ACHIEVEMENTS_PAGE,
    IN_GAME,
    GAME_OVER,
    EXIT_REQUESTED
};

enum class GameMode { SINGLE_PLAYER,
                      MULTIPLAYER };

struct GameSettings {
    GameMode gameMode = GameMode::SINGLE_PLAYER;
    int gameSpeedIndex = 1; // 0-based index into predefined speed options
    double speedMultiplier = 1.0;
    std::string gameSpeedLabel = "Normal";
    bool wrapAroundEdges = false;
    int boardWidth = 15;
    int boardHeight = 10;
    bool additionalFoodItems = false;

    // Graphics settings (can be extended per library)
    bool useAlternativeColors = false;
    bool showBorders = true;
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
        applySpeedOption(_settings.gameSpeedIndex);
    }

    const std::vector<MenuItem>& getCurrentMenuItems() const;
    int getCurrentSelection() const {
        return _currentSelection;
    }
    std::string getCurrentTitle() const;

    std::vector<std::string> getCreditsContent() const;
    std::vector<std::string> getInstructionsContent() const;
    std::vector<std::string> getAchievementsContent(const game_data& game) const;
    std::vector<std::string> getSettingsContent() const;

    void toggleGameMode();
    void adjustGameSpeed(int delta);
    void toggleWrapAround();
    void toggleAdditionalFoodItems();
    void adjustBoardSize(int widthDelta, int heightDelta);
    void toggleAlternativeColors();
    void toggleGrid();
    void toggleFPS();

    void setGameOverScore(int score);
    int getGameOverScore() const;

    // Feature availability toggles (from engine)
    void setBonusFeaturesAvailable(bool enabled) { _bonusFeaturesAvailable = enabled; }
    bool isBonusFeaturesAvailable() const { return _bonusFeaturesAvailable; }

  private:
    MenuState _currentState;
    int _currentSelection;
    GameSettings _settings;
    int _gameOverScore;
    bool _bonusFeaturesAvailable = false;

    std::vector<MenuItem> _mainMenuItems;
    std::vector<MenuItem> _settingsMenuItems;
    std::vector<MenuItem> _gameOverMenuItems;

    void initializeMenus();
    void updateSettingsMenu();
    void clampSelection();
    void applySpeedOption(int index);
};
