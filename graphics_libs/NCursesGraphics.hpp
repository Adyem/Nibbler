#ifndef NCURSESGRAPHICS_HPP
#define NCURSESGRAPHICS_HPP

#include "../IGraphicsLibrary.hpp"
#include "../MenuSystem.hpp"
#include <ncurses.h>
#include <string>

class NCursesGraphics : public IGraphicsLibrary {
public:
    NCursesGraphics();
    virtual ~NCursesGraphics();

    // IGraphicsLibrary interface implementation
    virtual int initialize() override;
    virtual void shutdown() override;
    virtual void render(const game_data& game) override;
    virtual GameKey getInput() override;
    virtual const char* getName() const override;
    virtual bool shouldContinue() const override;
    virtual void setFrameRate(int fps) override;
    virtual const char* getError() const override;

    // Menu system support (override from IGraphicsLibrary)
    virtual void setMenuSystem(MenuSystem* menuSystem) override { _menuSystem = menuSystem; }
    MenuSystem* getMenuSystem() const { return _menuSystem; }

    // Switch message support (override from IGraphicsLibrary)
    virtual void setSwitchMessage(const std::string& message, int timer) override;
    bool hasSwitchMessage() const { return _switchMessageTimer > 0; }

    // Force input readiness
    void forceInputReadiness();

private:
    bool _initialized;
    bool _shouldContinue;
    int _frameRate;
    std::string _errorMessage;
    WINDOW* _gameWindow;

    // Switch message display
    std::string _switchMessage;
    int _switchMessageTimer;
    WINDOW* _infoWindow;
    MenuSystem* _menuSystem;

    // Color pairs
    enum ColorPairs {
        COLOR_SNAKE_HEAD = 1,
        COLOR_SNAKE_BODY = 2,
        COLOR_FOOD = 3,
        COLOR_WALL = 4,
        COLOR_ICE = 5,
        COLOR_BORDER = 6,
        COLOR_INFO = 7
    };

    // Helper methods
    void initializeColors();
    void drawBorder(const game_data& game);
    void drawGameArea(const game_data& game);
    void drawInfo(const game_data& game);
    char getCharFromGameTile(int x, int y, const game_data& game);
    int getColorFromGameTile(int x, int y, const game_data& game);
    void setError(const std::string& error);
    void clearError();

    // Menu rendering methods
    void renderMenu(const game_data& game);
    void renderMainMenu();
    void renderSettingsMenu();
    void renderCreditsPage();
    void renderInstructionsPage();
    void renderAchievementsPage(const game_data& game);
    void renderGameOverScreen();
    void drawCenteredText(int y, const std::string& text, int colorPair = 0);
    void drawMenuItems(const std::vector<MenuItem>& items, int selection, int startY);
};

#endif // NCURSESGRAPHICS_HPP