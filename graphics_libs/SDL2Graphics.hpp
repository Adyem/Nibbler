#ifndef SDL2GRAPHICS_HPP
#define SDL2GRAPHICS_HPP

#include "../IGraphicsLibrary.hpp"
#include "../MenuSystem.hpp"
#include <SDL.h>
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

    // Menu system
    MenuSystem* _menuSystem;

    // Switch message display
    std::string _switchMessage;
    int _switchMessageTimer;

    // Window dimensions
    static const int WINDOW_WIDTH = 800;
    static const int WINDOW_HEIGHT = 600;
    static const int CELL_SIZE = 20;

    // Colors (RGB values)
    struct Color {
        Uint8 r, g, b, a;
        Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha = 255) 
            : r(red), g(green), b(blue), a(alpha) {}
    };

    static const Color COLOR_BACKGROUND;
    static const Color COLOR_BORDER;
    static const Color COLOR_SNAKE_HEAD;
    static const Color COLOR_SNAKE_BODY;
    static const Color COLOR_FOOD;
    static const Color COLOR_TEXT;

    // Helper methods
    void setError(const std::string& error);
    void clearError();
    void setDrawColor(const Color& color);
    void drawRect(int x, int y, int width, int height, bool filled = true);
    void drawText(const std::string& text, int x, int y);
    GameKey translateSDLKey(SDL_Keycode key);
    void calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize);

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

#endif // SDL2GRAPHICS_HPP
