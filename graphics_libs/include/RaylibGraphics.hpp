#ifndef RAYLIBGRAPHICS_HPP
#define RAYLIBGRAPHICS_HPP

#include "../../IGraphicsLibrary.hpp"
#include "../../MenuSystem.hpp"
#include <string>
#include <vector>

// Forward declare to avoid including raylib everywhere in headers
struct Font; // not used directly; raylib manages default font internally

class RaylibGraphics : public IGraphicsLibrary {
  public:
    RaylibGraphics();
    ~RaylibGraphics() override;

    int initialize() override;
    void shutdown() override;
    void render(const game_data& game) override;
    GameKey getInput() override;
    const char* getName() const override;
    bool shouldContinue() const override;
    void setFrameRate(int fps) override;
    const char* getError() const override;
    void setMenuSystem(MenuSystem* menuSystem) override;
    void setSwitchMessage(const std::string& message, int timer) override;

  private:
    struct Color {
        unsigned char r, g, b, a;
        Color(unsigned char r = 255, unsigned char g = 255, unsigned char b = 255, unsigned char a = 255) : r(r), g(g), b(b), a(a) {}
    };

    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;

    // Colors
    static const Color COLOR_BACKGROUND;
    static const Color COLOR_BORDER;
    static const Color COLOR_SNAKE_HEAD;
    static const Color COLOR_SNAKE_BODY;
    static const Color COLOR_FOOD;
    static const Color COLOR_TEXT;
    static const Color COLOR_SELECTOR_BG;
    static const Color COLOR_SELECTED_TEXT;

    bool _initialized;
    bool _shouldContinue;
    int _targetFPS;

    MenuSystem* _menuSystem;
    std::string _switchMessage;
    int _switchMessageTimer;

    std::string _errorMessage;

    // Helpers
    void setError(const std::string& msg);
    void clearError();

    void calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize);
    void drawRect(int x, int y, int w, int h, const Color& color, bool filled = true);
    void drawText(const std::string& text, int x, int y, const Color& color, int size);
    void drawCenteredText(const std::string& text, int y, const Color& color, int size);

    // Menu renderers
    void renderMenu();
    void renderMainMenu();
    void renderSettingsMenu();
    void renderCreditsMenu();
    void renderInstructionsMenu();
    void renderGameOverMenu();
    void drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY);
};

extern "C" {
IGraphicsLibrary* createGraphicsLibrary();
void destroyGraphicsLibrary(IGraphicsLibrary* lib);
const char* getLibraryName();
const char* getLibraryVersion();
}

#endif // RAYLIBGRAPHICS_HPP
