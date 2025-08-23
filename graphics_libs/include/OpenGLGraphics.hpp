#pragma once

#include "../../IGraphicsLibrary.hpp"
#include "../../MenuSystem.hpp"
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

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
    
    // Private helper methods
    void clearError();
    void setError(const std::string& error);
    GameKey translateGLFWKey(int key);
    void calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize);
    void drawRectangle(int x, int y, int width, int height, const Color& color);
    void drawText(const std::string& text, int x, int y, const Color& color, float scale = 1.0f);
    void renderMainMenu();
    void renderSettingsMenu();
    void renderGameOverScreen();
    void renderCreditsPage();
    void renderInstructionsPage();
    
    // Font rendering (simple bitmap font)
    void renderBitmapChar(char c, int x, int y, const Color& color, float scale);
    
    // GLFW callbacks
    static void errorCallback(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
