#pragma once

#include "../../IGraphicsLibrary.hpp"
#include "../../MenuSystem.hpp"
#include <ncurses.h>
#include <string>

class NCursesGraphics : public IGraphicsLibrary {
  public:
    NCursesGraphics();
    virtual ~NCursesGraphics();

    virtual int initialize() override;
    virtual void shutdown() override;
    virtual void render(const game_data& game) override;
    virtual GameKey getInput() override;
    virtual const char* getName() const override;
    virtual bool shouldContinue() const override;
    virtual void setFrameRate(int fps) override;
    virtual const char* getError() const override;

    virtual void setMenuSystem(MenuSystem* menuSystem) override {
        _menuSystem = menuSystem;
    }
    MenuSystem* getMenuSystem() const {
        return _menuSystem;
    }

    virtual void setSwitchMessage(const std::string& message, int timer) override;
    bool hasSwitchMessage() const {
        return _switchMessageTimer > 0;
    }

    void forceInputReadiness();

  private:
    bool _initialized;
    bool _shouldContinue;
    int _frameRate;
    std::string _errorMessage;
    WINDOW* _gameWindow;

    std::string _switchMessage;
    int _switchMessageTimer;
    WINDOW* _infoWindow;
    MenuSystem* _menuSystem;

    // Track active palette for ncurses color pairs
    bool _altColorsActive = false;

    enum ColorPairs { COLOR_SNAKE_HEAD = 1,
                      COLOR_SNAKE_BODY,
                      COLOR_FOOD,
                      COLOR_WALL,
                      COLOR_ICE,
                      COLOR_BORDER,
                      COLOR_INFO };

    void initializeColors();
    void drawBorder(const game_data& game);
    void drawGameArea(const game_data& game);
    void drawInfo(const game_data& game);
    char getCharFromGameTile(int x, int y, const game_data& game);
    int getColorFromGameTile(int x, int y, const game_data& game);
    void setError(const std::string& error);
    void clearError();
    void renderMenu();
    void renderMainMenu();
    void renderSettingsMenu();
    void renderCreditsPage();
    void renderInstructionsPage();
    void renderGameOverScreen();
    void drawCenteredText(int y, const std::string& text, int colorPair = 0);
    void drawMenuItems(const std::vector<MenuItem>& items, int selection, int startY);
};
