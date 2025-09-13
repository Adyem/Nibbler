#pragma once

#include "../../IGraphicsLibrary.hpp"
#include "../../MenuSystem.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>

// FreeType forward declarations
// We include these here (instead of only in the .cpp) because we store FT types as members.
#include <ft2build.h>
#include FT_FREETYPE_H

class OpenGLGraphics : public IGraphicsLibrary {
  public:
    OpenGLGraphics();
    virtual ~OpenGLGraphics();

    // IGraphicsLibrary interface implementation
    virtual int initialize() override;
    virtual void shutdown() override;
    virtual void render(const game_data& game) override;
    virtual GameKey getInput() override;
    bool shouldContinue() const override;
    const char* getError() const override;
    const char* getName() const override;
    void setFrameRate(int fps) override;
    virtual void setMenuSystem(MenuSystem* menuSystem) override;
    virtual void setSwitchMessage(const std::string& message, int duration) override;

  private:
    // Window and rendering
    GLFWwindow* _window;
    bool _initialized;
    bool _shouldContinue;
    int _targetFPS;

    // Menu system
    MenuSystem* _menuSystem;

    // Error handling
    std::string _errorMessage;

    // Switch message system
    std::string _switchMessage;
    int _switchMessageTimer;

    // Input handling
    GameKey _lastKeyPressed;
    bool _keyConsumed;

  // --- Font / text rendering ---
  struct Glyph {
    unsigned int textureId; // OpenGL texture for the glyph bitmap
    int width;
    int height;
    int bearingX; // Left bearing
    int bearingY; // Top bearing
    long advance; // Advance (in 1/64 pixels from FreeType)
    unsigned int index; // FreeType glyph index (for kerning)
  };

  bool _fontInitialized;
  int _fontPixelSize = 24;
  FT_Library _ftLibrary = nullptr;
  FT_Face _ftFace = nullptr;
  std::map<char, Glyph> _glyphs; // Basic ASCII glyph cache

    // Window dimensions
    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;

    // Colors (as RGB floats 0.0-1.0)
    struct Color {
        float r, g, b, a;
        Color(float red, float green, float blue, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {}
    };

    // Color palette
    static const Color COLOR_BACKGROUND;
    static const Color COLOR_BORDER;
    static const Color COLOR_SNAKE_HEAD;
    static const Color COLOR_SNAKE_BODY;
    static const Color COLOR_FOOD;
    static const Color COLOR_TEXT;
    static const Color COLOR_SELECTOR_BG;
    static const Color COLOR_SELECTED_TEXT;
    // Extra items/tiles
    static const Color COLOR_FIRE_FOOD;
    static const Color COLOR_FROSTY_FOOD;
    static const Color COLOR_FIRE_TILE;

    // Alternative palette
    static const Color ALT_COLOR_BACKGROUND;
    static const Color ALT_COLOR_BORDER;
    static const Color ALT_COLOR_SNAKE_HEAD;
    static const Color ALT_COLOR_SNAKE_BODY;
    static const Color ALT_COLOR_FOOD;
    static const Color ALT_COLOR_TEXT;

    // Private helper methods
    void clearError();
    void setError(const std::string& error);
    GameKey translateGLFWKey(int key);
    void calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize);
    void drawRectangle(int x, int y, int width, int height, const Color& color);
    void drawText(const std::string& text, int x, int y, const Color& color, float scale = 1.0f);
    int  measureTextWidth(const std::string& text, float scale = 1.0f) const;
    void drawCenteredText(const std::string& text, int y, const Color& color, float scale = 1.0f);
    void drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY, float scale = 1.0f);
  void renderChar(char c, float x, float y, float scale, const Color& color);

  // Font helpers
  bool initializeFonts();
  void shutdownFonts();
  bool loadFontFace(const std::string& path);
  bool buildGlyphCache();
  void renderText(const std::string& text, float x, float y, float scale, const Color& color);

    void renderMainMenu();
    void renderSettingsMenu();
    void renderGameOverScreen();
    void renderCreditsPage();
    void renderInstructionsPage();
    void renderAchievementsPage(const game_data& game);

    // GLFW callbacks
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
