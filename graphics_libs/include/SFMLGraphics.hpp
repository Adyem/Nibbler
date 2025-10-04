#pragma once

#include "../../IGraphicsLibrary.hpp"
#include "../../MenuSystem.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class SFMLGraphics : public IGraphicsLibrary {
  public:
    SFMLGraphics();
    virtual ~SFMLGraphics();

    // IGraphicsLibrary interface implementation
    virtual int initialize() override;
    virtual void shutdown() override;
    virtual void render(const game_data& game) override;
    virtual GameKey getInput() override;
    virtual const char* getName() const override;
    virtual bool shouldContinue() const override;
    virtual const char* getError() const override;
    virtual void setFrameRate(int fps) override;
    virtual void setMenuSystem(MenuSystem* menuSystem) override;
    virtual void setSwitchMessage(const std::string& message, int timer) override;

  private:
    bool _initialized;
    bool _shouldContinue;
    std::string _errorMessage;
    int _targetFPS;

    // SFML objects
    sf::RenderWindow* _window;
    sf::Font _font;

    // Menu system
    MenuSystem* _menuSystem;

    // Switch message display
    std::string _switchMessage;
    int _switchMessageTimer;

    // Window dimensions
    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;
    static const int CELL_SIZE = 20;

    // Colors
    struct Color {
        unsigned char r, g, b, a;
        Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 255)
            : r(red), g(green), b(blue), a(alpha) {}

        sf::Color toSFColor() const {
            return sf::Color(r, g, b, a);
        }
    };

    // Default palette
    static const Color COLOR_BACKGROUND;
    static const Color COLOR_BORDER;
    static const Color COLOR_SNAKE_HEAD;
    static const Color COLOR_SNAKE_BODY;
    static const Color COLOR_FOOD;
    static const Color COLOR_TEXT;
    static const Color COLOR_FIRE_FOOD;
    static const Color COLOR_FROSTY_FOOD;
    static const Color COLOR_FIRE_TILE;
    static const Color COLOR_SELECTOR_BG;
    static const Color COLOR_SELECTED_TEXT;

    // Alternative palette
    static const Color ALT_COLOR_BACKGROUND;
    static const Color ALT_COLOR_BORDER;
    static const Color ALT_COLOR_SNAKE_HEAD;
    static const Color ALT_COLOR_SNAKE_BODY;
    static const Color ALT_COLOR_FOOD;
    static const Color ALT_COLOR_TEXT;
    static const Color ALT_COLOR_FIRE_FOOD;
    static const Color ALT_COLOR_FROSTY_FOOD;
    static const Color ALT_COLOR_FIRE_TILE;

    // Helper methods
    void setError(const std::string& error);
    void clearError();
    void drawRect(int x, int y, int width, int height, const Color& color, bool filled = true);
    void drawText(const std::string& text, int x, int y, const Color& color = Color(255, 255, 255), int fontSize = 16);
    GameKey translateSFMLKey(sf::Keyboard::Key key);
    void calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize);

    // Font methods
    bool initializeFont();
    void drawCenteredText(const std::string& text, int y, const Color& color = Color(255, 255, 255), int fontSize = 24);
    int getTextWidth(const std::string& text, int fontSize = 16);
    int getTextHeight(int fontSize = 16);

    // Menu rendering methods
    void renderMenu();
    void renderMainMenu();
    void renderSettingsMenu();
    void renderCreditsMenu();
    void renderInstructionsMenu();
    void renderGameOverMenu();
    void drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY);
};
