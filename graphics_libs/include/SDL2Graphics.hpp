#pragma once

#include "../../IGraphicsLibrary.hpp"
#include "../../MenuSystem.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class SDL2Graphics : public IGraphicsLibrary {
  public:
    SDL2Graphics();
    virtual ~SDL2Graphics();

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
    Uint32 _frameDelay;

    // SDL objects
    SDL_Window* _window;
    SDL_Renderer* _renderer;

    // Font objects
    TTF_Font* _fontLarge;  // For titles
    TTF_Font* _fontMedium; // For menu items
    TTF_Font* _fontSmall;  // For instructions

    // Menu system
    MenuSystem* _menuSystem;

    // Switch message display
    std::string _switchMessage;
    int _switchMessageTimer;

    // Window dimensions
    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;
    static const int CELL_SIZE = 20;

    // Colors (RGB values)
    struct Color {
        Uint8 r, g, b, a;
        Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255)
            : r(red), g(green), b(blue), a(alpha) {}
    };

    // Default palette
    static const Color COLOR_BACKGROUND;
    static const Color COLOR_BORDER;
    static const Color COLOR_SNAKE_HEAD;
    static const Color COLOR_SNAKE_BODY;
    static const Color COLOR_FOOD;
    static const Color COLOR_TEXT;
    static const Color COLOR_SELECTOR_BG;
    static const Color COLOR_SELECTED_TEXT;

    // Alternative palette
    static const Color ALT_COLOR_BACKGROUND;
    static const Color ALT_COLOR_BORDER;
    static const Color ALT_COLOR_SNAKE_HEAD;
    static const Color ALT_COLOR_SNAKE_BODY;
    static const Color ALT_COLOR_FOOD;
    static const Color ALT_COLOR_TEXT;

    // Helper methods
    void setError(const std::string& error);
    void clearError();
    void setDrawColor(const Color& color);
    void drawRect(int x, int y, int width, int height, bool filled = true);
    void drawTransparentRect(int x, int y, int width, int height, const Color& color, Uint8 alpha);
    void drawText(const std::string& text, int x, int y);
    GameKey translateSDLKey(SDL_Keycode key);
    void calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize);

    // Font methods
    bool initializeFonts();
    void shutdownFonts();
    void drawTextWithFont(const std::string& text, int x, int y, TTF_Font* font, const Color& color = COLOR_TEXT);
    void drawCenteredTextWithFont(const std::string& text, int y, TTF_Font* font, const Color& color = COLOR_TEXT);
    int getTextWidth(const std::string& text, TTF_Font* font);
    int getTextHeight(TTF_Font* font);

    // Menu rendering methods
    void renderMenu(const game_data& game);
    void renderMainMenu();
    void renderSettingsMenu();
    void renderCreditsMenu();
    void renderInstructionsMenu();
    void renderAchievementsMenu(const game_data& game);
    void renderGameOverMenu();
    void drawCenteredText(const std::string& text, int y, const Color& color = COLOR_TEXT);
    void drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY);
};
