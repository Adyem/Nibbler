#include "OpenGLGraphics.hpp"
#include "../../game_data.hpp"
#include "../../MenuSystem.hpp"
#include <GL/gl.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <filesystem>

// Helper macro to safely delete OpenGL textures
static void safeDeleteTexture(unsigned int &tex) {
    if (tex) {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
}

// Static color definitions (RGB values 0.0-1.0)
const OpenGLGraphics::Color OpenGLGraphics::COLOR_BACKGROUND(0.08f, 0.08f, 0.12f);  // Dark blue-gray
const OpenGLGraphics::Color OpenGLGraphics::COLOR_BORDER(0.39f, 0.39f, 0.47f);      // Light gray
const OpenGLGraphics::Color OpenGLGraphics::COLOR_SNAKE_HEAD(0.20f, 0.78f, 0.20f);  // Bright green
const OpenGLGraphics::Color OpenGLGraphics::COLOR_SNAKE_BODY(0.12f, 0.59f, 0.12f);  // Dark green
const OpenGLGraphics::Color OpenGLGraphics::COLOR_FOOD(0.78f, 0.20f, 0.20f);        // Red
const OpenGLGraphics::Color OpenGLGraphics::COLOR_TEXT(1.0f, 1.0f, 1.0f);           // White
const OpenGLGraphics::Color OpenGLGraphics::COLOR_SELECTOR_BG(0.27f, 0.51f, 0.71f); // Steel blue
const OpenGLGraphics::Color OpenGLGraphics::COLOR_SELECTED_TEXT(1.0f, 1.0f, 1.0f);  // White

OpenGLGraphics::OpenGLGraphics()
        : _window(nullptr), _initialized(false), _shouldContinue(true), _targetFPS(60),
            _menuSystem(nullptr), _switchMessageTimer(0), _lastKeyPressed(GameKey::NONE), _keyConsumed(true),
            _fontInitialized(false) {
}

OpenGLGraphics::~OpenGLGraphics() {
    shutdown();
}

int OpenGLGraphics::initialize() {
    if (_initialized) {
        return 0;
    }

    clearError();

    // Set GLFW error callback
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW
    if (!glfwInit()) {
        setError("Failed to initialize GLFW");
        return 1;
    }

    // Configure GLFW window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create window
    _window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Nibbler - OpenGL Graphics", nullptr, nullptr);
    if (!_window) {
        setError("Failed to create GLFW window");
        glfwTerminate();
        return 1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(_window);

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(_window, this);

    // Set key callback
    glfwSetKeyCallback(_window, keyCallback);

    // Enable VSync
    glfwSwapInterval(1);

    // Set up OpenGL viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Set up 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1); // Top-left origin

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Enable basic OpenGL features
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize fonts (attempt). We don't hard fail the whole library if font init fails; we just log.
    if (!initializeFonts()) {
        std::cerr << "[OpenGLGraphics] Warning: Failed to initialize TTF font rendering. Text will be unavailable." << std::endl;
    }

    _initialized = true;
    _shouldContinue = true;

    return 0;
}

void OpenGLGraphics::shutdown() {
    if (!_initialized) {
        return;
    }

    _shouldContinue = false;

    // Shutdown fonts
    shutdownFonts();

    if (_window) {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }

    glfwTerminate();

    // Reset state
    _menuSystem = nullptr;
    _switchMessage.clear();
    _switchMessageTimer = 0;
    _targetFPS = 60;
    _initialized = false;

    clearError();
}

void OpenGLGraphics::render(const game_data& game) {
    if (!_initialized || !_window) {
        return;
    }

    // Clear the screen
    glClearColor(COLOR_BACKGROUND.r, COLOR_BACKGROUND.g, COLOR_BACKGROUND.b, COLOR_BACKGROUND.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // Handle different menu states
    if (_menuSystem) {
        switch (_menuSystem->getCurrentState()) {
        case MenuState::MAIN_MENU:
            renderMainMenu();
            break;
        case MenuState::SETTINGS_MENU:
            renderSettingsMenu();
            break;
        case MenuState::CREDITS_PAGE:
            renderCreditsPage();
            break;
        case MenuState::INSTRUCTIONS_PAGE:
            renderInstructionsPage();
            break;
        case MenuState::GAME_OVER:
            renderGameOverScreen();
            break;
        case MenuState::EXIT_REQUESTED:
            // Handle exit - could show a "closing..." message
            break;
        case MenuState::IN_GAME:
            // Render game
            {
                int offsetX, offsetY, cellSize;
                calculateGameArea(game, offsetX, offsetY, cellSize);

                // Draw game border
                drawRectangle(offsetX - 2, offsetY - 2, game.get_width() * cellSize + 4, game.get_height() * cellSize + 4, COLOR_BORDER);

                // Draw game board
                for (size_t y = 0; y < game.get_height(); ++y) {
                    for (size_t x = 0; x < game.get_width(); ++x) {
                        int drawX = offsetX + x * cellSize;
                        int drawY = offsetY + y * cellSize;

                        // Check layer 2 first (snake and food)
                        int layer2Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 2);
                        if (layer2Value == FOOD) {
                            drawRectangle(drawX + 2, drawY + 2, cellSize - 4, cellSize - 4, COLOR_FOOD);
                        } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
                            drawRectangle(drawX, drawY, cellSize, cellSize, COLOR_SNAKE_HEAD);
                        } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
                            drawRectangle(drawX, drawY, cellSize, cellSize, COLOR_SNAKE_BODY);
                        } else {
                            // Check layer 0 (terrain)
                            int layer0Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 0);
                            if (layer0Value == GAME_TILE_WALL) {
                                drawRectangle(drawX, drawY, cellSize, cellSize, COLOR_BORDER);
                            }
                            // Empty space and ice - no drawing needed (background already drawn)
                        }
                    }
                }

                // Draw score
                std::string scoreText = "Length: " + std::to_string(game.get_snake_length(0));
                drawText(scoreText, 20, 20, COLOR_TEXT);
            }
            break;
        }
    }

    // Draw switch message if active
    if (_switchMessageTimer > 0) {
        drawText(_switchMessage, WINDOW_WIDTH / 2 - (_switchMessage.length() * 6), 50, COLOR_TEXT, 1.5f);
        _switchMessageTimer--;
    }

    // Swap buffers and poll events
    glfwSwapBuffers(_window);
    glfwPollEvents();

    // Check if window should close
    if (glfwWindowShouldClose(_window)) {
        _shouldContinue = false;
    }
}

GameKey OpenGLGraphics::getInput() {
    if (!_initialized || !_window) {
        return GameKey::NONE;
    }

    // Check if we have a key that hasn't been consumed yet
    if (!_keyConsumed) {
        _keyConsumed = true;
        return _lastKeyPressed;
    }

    return GameKey::NONE;
}

bool OpenGLGraphics::shouldContinue() const {
    return _shouldContinue && _initialized;
}

const char* OpenGLGraphics::getName() const {
    return "OpenGL Graphics Library";
}

const char* OpenGLGraphics::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

void OpenGLGraphics::setFrameRate(int fps) {
    _targetFPS = fps;
}

void OpenGLGraphics::setMenuSystem(MenuSystem* menuSystem) {
    _menuSystem = menuSystem;
}

void OpenGLGraphics::setSwitchMessage(const std::string& message, int duration) {
    _switchMessage = message;
    _switchMessageTimer = duration;
}

// Private helper methods

void OpenGLGraphics::clearError() {
    _errorMessage.clear();
}

void OpenGLGraphics::setError(const std::string& error) {
    _errorMessage = error;
}

GameKey OpenGLGraphics::translateGLFWKey(int key) {
    switch (key) {
    case GLFW_KEY_UP:
        return GameKey::UP;
    case GLFW_KEY_DOWN:
        return GameKey::DOWN;
    case GLFW_KEY_LEFT:
        return GameKey::LEFT;
    case GLFW_KEY_RIGHT:
        return GameKey::RIGHT;
    case GLFW_KEY_1:
        return GameKey::KEY_1;
    case GLFW_KEY_2:
        return GameKey::KEY_2;
    case GLFW_KEY_3:
        return GameKey::KEY_3;
    case GLFW_KEY_4:
        return GameKey::KEY_4;
    case GLFW_KEY_ESCAPE:
        return GameKey::ESCAPE;
    default:
        return GameKey::NONE;
    }
}

void OpenGLGraphics::calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize) {
    size_t gameWidth = game.get_width();
    size_t gameHeight = game.get_height();

    int maxCellWidth = (WINDOW_WIDTH - 40) / static_cast<int>(gameWidth);
    int maxCellHeight = (WINDOW_HEIGHT - 100) / static_cast<int>(gameHeight);

    cellSize = std::min(maxCellWidth, maxCellHeight);
    cellSize = std::max(cellSize, 8); // Minimum cell size

    int totalGameWidth = gameWidth * cellSize;
    int totalGameHeight = gameHeight * cellSize;

    offsetX = (WINDOW_WIDTH - totalGameWidth) / 2;
    offsetY = (WINDOW_HEIGHT - totalGameHeight) / 2;
}

void OpenGLGraphics::drawRectangle(int x, int y, int width, int height, const Color& color) {
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void OpenGLGraphics::drawText(const std::string& text, int x, int y, const Color& color, float scale) {
    if (!_fontInitialized) return;
    renderText(text, static_cast<float>(x), static_cast<float>(y), scale, color);
}

// ----------------- Font Rendering Implementation -----------------

bool OpenGLGraphics::initializeFonts() {
    if (_fontInitialized) return true;

    if (FT_Init_FreeType(&_ftLibrary)) {
        std::cerr << "[OpenGLGraphics] FreeType init failed" << std::endl;
        return false;
    }

    // Candidate font paths (reuse typical system fonts). Adjust list as needed.
    const char* candidates[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf" // macOS fallback
    };

    bool faceLoaded = false;
    for (const char* path : candidates) {
        if (std::filesystem::exists(path) && loadFontFace(path)) {
            faceLoaded = true;
            break;
        }
    }

    if (!faceLoaded) {
        std::cerr << "[OpenGLGraphics] No suitable font file found" << std::endl;
        FT_Done_FreeType(_ftLibrary);
        _ftLibrary = nullptr;
        return false;
    }

    if (!buildGlyphCache()) {
        std::cerr << "[OpenGLGraphics] Failed building glyph cache" << std::endl;
        shutdownFonts();
        return false;
    }

    _fontInitialized = true;
    return true;
}

bool OpenGLGraphics::loadFontFace(const std::string& path) {
    if (FT_New_Face(_ftLibrary, path.c_str(), 0, &_ftFace)) {
        return false;
    }
    if (FT_Set_Pixel_Sizes(_ftFace, 0, _fontPixelSize)) {
        FT_Done_Face(_ftFace);
        _ftFace = nullptr;
        return false;
    }
    std::cout << "[OpenGLGraphics] Loaded font: " << path << std::endl;
    return true;
}

bool OpenGLGraphics::buildGlyphCache() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (unsigned char c = 32; c < 127; ++c) { // Basic printable ASCII
        if (FT_Load_Char(_ftFace, c, FT_LOAD_RENDER)) {
            std::cerr << "[OpenGLGraphics] Failed to load glyph '" << c << "'" << std::endl;
            continue; // Skip but keep going
        }

        FT_GlyphSlot g = _ftFace->glyph;

        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_ALPHA,
            g->bitmap.width,
            g->bitmap.rows,
            0,
            GL_ALPHA,
            GL_UNSIGNED_BYTE,
            g->bitmap.buffer
        );

        // Texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Glyph glyph;
        glyph.textureId = tex;
        glyph.width = g->bitmap.width;
        glyph.height = g->bitmap.rows;
        glyph.bearingX = g->bitmap_left;
        glyph.bearingY = g->bitmap_top;
        glyph.advance = g->advance.x; // 1/64 pixels

        _glyphs[static_cast<char>(c)] = glyph;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return !_glyphs.empty();
}

void OpenGLGraphics::shutdownFonts() {
    if (!_fontInitialized && !_ftLibrary) return;

    for (auto &kv : _glyphs) {
        safeDeleteTexture(kv.second.textureId);
    }
    _glyphs.clear();

    if (_ftFace) {
        FT_Done_Face(_ftFace);
        _ftFace = nullptr;
    }
    if (_ftLibrary) {
        FT_Done_FreeType(_ftLibrary);
        _ftLibrary = nullptr;
    }
    _fontInitialized = false;
}

void OpenGLGraphics::renderChar(char c, float x, float y, float scale, const Color& color) {
    auto it = _glyphs.find(c);
    if (it == _glyphs.end()) return;
    const Glyph &g = it->second;

    float xpos = x + g.bearingX * scale;
    float ypos = y - (g.height - g.bearingY) * scale;
    float w = g.width * scale;
    float h = g.height * scale;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g.textureId);
    glColor4f(color.r, color.g, color.b, color.a);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(xpos,     ypos);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(xpos + w, ypos);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(xpos + w, ypos + h);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(xpos,     ypos + h);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void OpenGLGraphics::renderText(const std::string& text, float x, float y, float scale, const Color& color) {
    // Baseline y provided at top-left like other libs, adjust so characters render downward
    float cursorX = x;
    float baselineY = y + _fontPixelSize * scale; // shift so text appears below input y
    for (char c : text) {
        if (c == '\n') {
            cursorX = x;
            baselineY += _fontPixelSize * scale * 1.3f; // line spacing
            continue;
        }
        auto it = _glyphs.find(c);
        if (it == _glyphs.end()) {
            cursorX += _fontPixelSize * scale * 0.5f; // fallback advance
            continue;
        }
        renderChar(c, cursorX, baselineY, scale, color);
        cursorX += (it->second.advance >> 6) * scale; // convert from 1/64
    }
}

void OpenGLGraphics::renderMainMenu() {
    if (!_menuSystem) return;

    drawText("NIBBLER", WINDOW_WIDTH / 2 - 42, 100, COLOR_TEXT, 2.0f);
    drawText("Main Menu", WINDOW_WIDTH / 2 - 54, 200, COLOR_TEXT, 1.5f);

    // Menu items
    std::vector<std::string> menuItems = {"Start Game", "Settings", "Credits", "Exit"};
    int selectedIndex = _menuSystem->getCurrentSelection();

    for (size_t i = 0; i < menuItems.size(); ++i) {
        int itemY = 300 + i * 60;
        Color textColor = (i == static_cast<size_t>(selectedIndex)) ? COLOR_SELECTED_TEXT : COLOR_TEXT;

        if (i == static_cast<size_t>(selectedIndex)) {
            drawRectangle(WINDOW_WIDTH / 2 - 100, itemY - 5, 200, 30, COLOR_SELECTOR_BG);
        }

        drawText(menuItems[i], WINDOW_WIDTH / 2 - (menuItems[i].length() * 6), itemY, textColor);
    }
}

void OpenGLGraphics::renderSettingsMenu() {
    if (!_menuSystem) return;

    drawText("Settings", WINDOW_WIDTH / 2 - 48, 100, COLOR_TEXT, 2.0f);

    // Settings implementation would go here
    drawText("Settings menu - Under construction", WINDOW_WIDTH / 2 - 150, 300, COLOR_TEXT);
    drawText("Press ESC to go back", WINDOW_WIDTH / 2 - 100, 400, COLOR_TEXT);
}

void OpenGLGraphics::renderGameOverScreen() {
    if (!_menuSystem) return;

    drawText("GAME OVER", WINDOW_WIDTH / 2 - 54, 200, COLOR_TEXT, 2.0f);

    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawText(scoreText, WINDOW_WIDTH / 2 - (scoreText.length() * 6), 300, COLOR_TEXT);

    drawText("Press ESC to return to main menu", WINDOW_WIDTH / 2 - 150, 400, COLOR_TEXT);
}

void OpenGLGraphics::renderCreditsPage() {
    drawText("CREDITS", WINDOW_WIDTH / 2 - 42, 100, COLOR_TEXT, 2.0f);
    drawText("Nibbler Snake Game", WINDOW_WIDTH / 2 - 90, 200, COLOR_TEXT);
    drawText("OpenGL + GLFW Graphics Engine", WINDOW_WIDTH / 2 - 150, 250, COLOR_TEXT);
    drawText("Press ESC to go back", WINDOW_WIDTH / 2 - 100, 400, COLOR_TEXT);
}

void OpenGLGraphics::renderInstructionsPage() {
    drawText("INSTRUCTIONS", WINDOW_WIDTH / 2 - 72, 100, COLOR_TEXT, 2.0f);
    drawText("Use arrow keys to move the snake", WINDOW_WIDTH / 2 - 150, 200, COLOR_TEXT);
    drawText("Eat food to grow", WINDOW_WIDTH / 2 - 80, 250, COLOR_TEXT);
    drawText("Don't hit walls or yourself", WINDOW_WIDTH / 2 - 120, 300, COLOR_TEXT);
    drawText("Press 1,2,3,4 to switch graphics", WINDOW_WIDTH / 2 - 140, 350, COLOR_TEXT);
    drawText("Press ESC to go back", WINDOW_WIDTH / 2 - 100, 400, COLOR_TEXT);
}

// GLFW callbacks

void OpenGLGraphics::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void OpenGLGraphics::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode; // Unused parameter
    (void)mods;     // Unused parameter

    if (action != GLFW_PRESS) {
        return; // Only handle key press events
    }

    OpenGLGraphics* graphics = static_cast<OpenGLGraphics*>(glfwGetWindowUserPointer(window));
    if (!graphics) {
        return;
    }

    GameKey gameKey = graphics->translateGLFWKey(key);

    // Queue the key for the main game loop
    if (gameKey != GameKey::NONE) {
        graphics->_lastKeyPressed = gameKey;
        graphics->_keyConsumed = false;
    }

    // Handle menu navigation when not in game
    if (graphics->_menuSystem && graphics->_menuSystem->getCurrentState() != MenuState::IN_GAME) {
        switch (gameKey) {
        case GameKey::UP:
            graphics->_menuSystem->navigateUp();
            break;
        case GameKey::DOWN:
            graphics->_menuSystem->navigateDown();
            break;
        case GameKey::ESCAPE:
            graphics->_menuSystem->goBack();
            break;
        default:
            // Other keys (including library switching) are handled by main game loop
            break;
        }

        // Handle Enter/Space for menu selection
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            graphics->_menuSystem->selectCurrentItem();
        }
    }
}

// C-style functions for dynamic loading
extern "C" {
    IGraphicsLibrary* createGraphicsLibrary() {
        return new OpenGLGraphics();
    }

    void destroyGraphicsLibrary(IGraphicsLibrary* lib) {
        delete lib;
    }

    const char* getLibraryName() {
        return "OpenGL Graphics Library";
    }

    const char* getLibraryVersion() {
        return "1.0.0";
    }
}
