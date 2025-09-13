#include "OpenGLGraphics.hpp"
#include "../../game_data.hpp"
#include "../../MenuSystem.hpp"
#include <GL/gl.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cmath>

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
const OpenGLGraphics::Color OpenGLGraphics::COLOR_SELECTOR_BG(0.27f, 0.51f, 0.71f, 0.55f); // Semi-transparent steel blue
const OpenGLGraphics::Color OpenGLGraphics::COLOR_SELECTED_TEXT(1.0f, 1.0f, 1.0f);  // White
const OpenGLGraphics::Color OpenGLGraphics::COLOR_FIRE_FOOD(0.92f, 0.31f, 0.18f);
const OpenGLGraphics::Color OpenGLGraphics::COLOR_FROSTY_FOOD(0.31f, 0.78f, 0.92f);
const OpenGLGraphics::Color OpenGLGraphics::COLOR_FIRE_TILE(0.78f, 0.20f, 0.20f);

// Alternative palette
const OpenGLGraphics::Color OpenGLGraphics::ALT_COLOR_BACKGROUND(0.06f, 0.06f, 0.07f);
const OpenGLGraphics::Color OpenGLGraphics::ALT_COLOR_BORDER(0.70f, 0.63f, 0.35f);
const OpenGLGraphics::Color OpenGLGraphics::ALT_COLOR_SNAKE_HEAD(0.31f, 0.71f, 0.86f);
const OpenGLGraphics::Color OpenGLGraphics::ALT_COLOR_SNAKE_BODY(0.16f, 0.47f, 0.71f);
const OpenGLGraphics::Color OpenGLGraphics::ALT_COLOR_FOOD(0.92f, 0.51f, 0.14f);
const OpenGLGraphics::Color OpenGLGraphics::ALT_COLOR_TEXT(0.94f, 0.94f, 0.94f);

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

    // Clear the screen (respect alt palette if set in settings)
    bool useAltBg = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& bgc = useAltBg ? ALT_COLOR_BACKGROUND : COLOR_BACKGROUND;
    glClearColor(bgc.r, bgc.g, bgc.b, bgc.a);
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
        case MenuState::ACHIEVEMENTS_PAGE:
            renderAchievementsPage(game);
            break;
        case MenuState::EXIT_REQUESTED:
            // Handle exit - could show a "closing..." message
            break;
        case MenuState::IN_GAME:
            // Render game
            {
                int offsetX, offsetY, cellSize;
                calculateGameArea(game, offsetX, offsetY, cellSize);

                // Pick palette
                bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
                const Color& border = useAlt ? ALT_COLOR_BORDER : COLOR_BORDER;
                const Color& head   = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
                const Color& body   = useAlt ? ALT_COLOR_SNAKE_BODY : COLOR_SNAKE_BODY;
                const Color& food   = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;

                // Draw game border
                drawRectangle(offsetX - 2, offsetY - 2, game.get_width() * cellSize + 4, game.get_height() * cellSize + 4, border);

                // Draw game board
                for (size_t y = 0; y < game.get_height(); ++y) {
                    for (size_t x = 0; x < game.get_width(); ++x) {
                        int drawX = offsetX + x * cellSize;
                        int drawY = offsetY + y * cellSize;

                        // Check layer 2 first (snake and food)
                        int layer2Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 2);
                        if (layer2Value == FOOD) {
                            drawRectangle(drawX + 2, drawY + 2, cellSize - 4, cellSize - 4, food);
                        } else if (layer2Value == FIRE_FOOD) {
                            drawRectangle(drawX + 2, drawY + 2, cellSize - 4, cellSize - 4, COLOR_FIRE_FOOD);
                        } else if (layer2Value == FROSTY_FOOD) {
                            drawRectangle(drawX + 2, drawY + 2, cellSize - 4, cellSize - 4, COLOR_FROSTY_FOOD);
                        } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
                            drawRectangle(drawX, drawY, cellSize, cellSize, head);
                        } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
                            drawRectangle(drawX, drawY, cellSize, cellSize, body);
                        } else {
                            // Check layer 0 (terrain)
                            int layer0Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 0);
                            if (layer0Value == GAME_TILE_WALL) {
                                drawRectangle(drawX, drawY, cellSize, cellSize, border);
                            } else if (layer0Value == GAME_TILE_FIRE) {
                                drawRectangle(drawX, drawY, cellSize, cellSize, COLOR_FIRE_TILE);
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
    // Do not display any library switch message per requirements

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

int OpenGLGraphics::measureTextWidth(const std::string& text, float scale) const {
    if (!_fontInitialized) return static_cast<int>(text.size() * _fontPixelSize * 0.6f * scale);
    int width = 0;
    unsigned int prevIndex = 0;
    for (char c : text) {
        auto it = _glyphs.find(c);
        if (it == _glyphs.end()) {
            width += static_cast<int>(_fontPixelSize * 0.5f * scale);
            continue;
        }
        if (_ftFace && FT_HAS_KERNING(_ftFace) && prevIndex != 0) {
            FT_Vector delta;
            FT_Get_Kerning(_ftFace, prevIndex, it->second.index, FT_KERNING_DEFAULT, &delta);
            width += static_cast<int>((delta.x >> 6) * scale);
        }
        width += static_cast<int>((it->second.advance >> 6) * scale);
        prevIndex = it->second.index;
    }
    return width;
}

void OpenGLGraphics::drawCenteredText(const std::string& text, int y, const Color& color, float scale) {
    int w = measureTextWidth(text, scale);
    drawText(text, (WINDOW_WIDTH - w) / 2, y, color, scale);
}

void OpenGLGraphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY, float scale) {
    int y = startY;
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        if (item.text.empty()) { y += static_cast<int>(_fontPixelSize * scale * 0.6f); continue; }
        int textW = measureTextWidth(item.text, scale);
        int rectW = textW + 20;
        int rectH = static_cast<int>(_fontPixelSize * scale * 1.3f);
        int rx = (WINDOW_WIDTH - rectW) / 2;
        int ry = y - 5;
        bool selected = item.selectable && static_cast<int>(i) == selectedIndex;
        if (selected) {
            drawRectangle(rx, ry, rectW, rectH, COLOR_SELECTOR_BG);
        }
        drawText(item.text, (WINDOW_WIDTH - textW) / 2, y, selected ? COLOR_SELECTED_TEXT : COLOR_TEXT, scale);
        y += rectH;
    }
}

// ----------------- Font Rendering Implementation -----------------

bool OpenGLGraphics::initializeFonts() {
    if (_fontInitialized) return true;

    if (FT_Init_FreeType(&_ftLibrary)) {
        std::cerr << "[OpenGLGraphics] FreeType init failed" << std::endl;
        return false;
    }

    // Candidate font paths (prefer neutral sans, avoid DejaVu per request)
    const char* candidates[] = {
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
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
    // Avoid italic/cursive styles if possible
    if (_ftFace->style_flags & FT_STYLE_FLAG_ITALIC) {
        FT_Done_Face(_ftFace);
        _ftFace = nullptr;
        return false; // try next candidate
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
        glyph.index = g->glyph_index;

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

    float xpos = std::round(x + g.bearingX * scale);
    // Place top-left of bitmap at baseline minus bearingY (consistent baseline)
    float ypos = std::round(y - g.bearingY * scale);
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
    // Use face ascender/line height for consistent baseline and spacing
    float cursorX = std::round(x);
    float ascenderPx = (_ftFace && _ftFace->size) ? std::round(((_ftFace->size->metrics.ascender >> 6) * scale))
                                                 : std::round(_fontPixelSize * scale * 0.8f);
    float lineH = (_ftFace && _ftFace->size && _ftFace->size->metrics.height)
                    ? ((_ftFace->size->metrics.height >> 6) * scale)
                    : (_fontPixelSize * scale * 1.3f);
    float baselineY = std::round(y + ascenderPx);

    unsigned int prevIndex = 0;
    for (char c : text) {
        if (c == '\n') {
            cursorX = std::round(x);
            baselineY = std::round(baselineY + lineH);
            prevIndex = 0;
            continue;
        }
        auto it = _glyphs.find(c);
        if (it == _glyphs.end()) {
            cursorX += _fontPixelSize * scale * 0.5f; // fallback advance
            continue;
        }
        if (_ftFace && FT_HAS_KERNING(_ftFace) && prevIndex != 0) {
            FT_Vector delta;
            FT_Get_Kerning(_ftFace, prevIndex, it->second.index, FT_KERNING_DEFAULT, &delta);
            cursorX += (delta.x >> 6) * scale;
        }
        renderChar(c, cursorX, baselineY, scale, color);
        cursorX += (it->second.advance >> 6) * scale; // convert from 1/64
        prevIndex = it->second.index;
    }
}

void OpenGLGraphics::renderMainMenu() {
    if (!_menuSystem) return;
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, COLOR_TEXT, 2.0f);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 220, 1.0f);
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, COLOR_TEXT, 1.0f);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, COLOR_TEXT, 1.0f);
}

void OpenGLGraphics::renderSettingsMenu() {
    if (!_menuSystem) return;
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_TEXT, 2.0f);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 140, 1.0f);
    drawCenteredText("Use Arrow Keys to navigate, ENTER to toggle/adjust", WINDOW_HEIGHT - 100, COLOR_TEXT, 1.0f);
    drawCenteredText("ESC to go back", WINDOW_HEIGHT - 80, COLOR_TEXT, 1.0f);
}

void OpenGLGraphics::renderGameOverScreen() {
    if (!_menuSystem) return;
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_TEXT, 2.0f);
    drawCenteredText("Your snake has collided!", 120, COLOR_FOOD, 1.2f);
    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredText(scoreText, 160, COLOR_TEXT, 1.2f);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 220, 1.0f);
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, COLOR_TEXT, 1.0f);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, COLOR_TEXT, 1.0f);
}

void OpenGLGraphics::renderCreditsPage() {
    if (!_menuSystem) return;
    // Reduce title size further to fit more content
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_TEXT, 1.5f);
    const auto& content = _menuSystem->getCreditsContent();
    int top = 120;
    // Smaller content scale and tighter line spacing
    float contentScale = 0.85f;
    int lineH = static_cast<int>(_fontPixelSize * contentScale + 3);
    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;
    int bottomY = WINDOW_HEIGHT - 80;
    int bonusIdx = -1;
    for (size_t i = 0; i < content.size(); ++i) if (content[i] == "BONUS FEATURES:") { bonusIdx = static_cast<int>(i); break; }
    if (bonusIdx >= 0) {
        int y1 = top;
        for (int i = 0; i < bonusIdx && y1 <= bottomY - lineH; ++i) {
            if (content[i].empty()) { y1 += lineH; continue; }
            drawText(content[i], colX1, y1, COLOR_TEXT, contentScale); y1 += lineH;
        }
        int y2 = top;
        for (size_t i = bonusIdx; i < content.size() && y2 <= bottomY - lineH; ++i) {
            if (content[i].empty()) { y2 += lineH; continue; }
            drawText(content[i], colX2, y2, COLOR_TEXT, contentScale); y2 += lineH;
        }
    } else {
        int usableH = bottomY - top - 20;
        int linesPerCol = std::max(1, usableH / lineH);
        for (size_t i = 0; i < content.size(); ++i) {
            int col = static_cast<int>(i) / linesPerCol;
            int row = static_cast<int>(i) % linesPerCol;
            int x = (col % 2 == 0) ? colX1 : colX2;
            int y = top + row * lineH;
            if (y > bottomY) continue;
            drawText(content[i], x, y, COLOR_TEXT, contentScale);
        }
    }
    drawCenteredText("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 50, COLOR_TEXT, 1.0f);
}

void OpenGLGraphics::renderInstructionsPage() {
    if (!_menuSystem) return;
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_TEXT, 2.0f);
    const auto& content = _menuSystem->getInstructionsContent();
    int top = 120;
    int lineH = static_cast<int>(_fontPixelSize * 1.0f + 4);
    int usableH = WINDOW_HEIGHT - top - 100;
    int linesPerCol = std::max(1, usableH / lineH);
    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;
    for (size_t i = 0; i < content.size(); ++i) {
        int col = static_cast<int>(i) / linesPerCol;
        int row = static_cast<int>(i) % linesPerCol;
        int x = (col % 2 == 0) ? colX1 : colX2;
        int y = top + row * lineH;
        if (y > WINDOW_HEIGHT - 80) continue;
        drawText(content[i], x, y, COLOR_TEXT, 1.0f);
    }
    drawCenteredText("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 50, COLOR_TEXT, 1.0f);
}

void OpenGLGraphics::renderAchievementsPage(const game_data& game) {
    if (!_menuSystem) return;
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_TEXT, 2.0f);
    const auto& content = _menuSystem->getAchievementsContent(game);
    int top = 120;
    int lineH = static_cast<int>(_fontPixelSize * 1.0f + 4);
    int y = top;
    for (const auto& line : content) {
        if (y > WINDOW_HEIGHT - 80) break;
        drawCenteredText(line, y, COLOR_TEXT, 1.0f);
        y += lineH;
    }
    drawCenteredText("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 50, COLOR_TEXT, 1.0f);
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

    bool inMenu = graphics->_menuSystem && graphics->_menuSystem->getCurrentState() != MenuState::IN_GAME;

    // If in menu, handle navigation locally and do NOT propagate ESC/Enter/Space/Arrows
    if (inMenu) {
        // Allow library switching while in menu
        if (gameKey == GameKey::KEY_1 || gameKey == GameKey::KEY_2 ||
            gameKey == GameKey::KEY_3 || gameKey == GameKey::KEY_4) {
            graphics->_lastKeyPressed = gameKey;
            graphics->_keyConsumed = false;
            return;
        }

        switch (gameKey) {
        case GameKey::UP:
            graphics->_menuSystem->navigateUp();
            return;
        case GameKey::DOWN:
            graphics->_menuSystem->navigateDown();
            return;
        case GameKey::ESCAPE:
            graphics->_menuSystem->goBack();
            return;
        default:
            break;
        }

        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) {
            graphics->_menuSystem->selectCurrentItem();
            return;
        }

        // Ignore other keys in menu
        return;
    }

    // Not in menu: queue the key for main loop
    if (gameKey != GameKey::NONE) {
        graphics->_lastKeyPressed = gameKey;
        graphics->_keyConsumed = false;
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
