#include "RaylibGraphics.hpp"
#include "../../game_data.hpp"
#include "../../MenuSystem.hpp"
#include <raylib.h>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>

const RaylibGraphics::Color RaylibGraphics::COLOR_BACKGROUND(20, 20, 30, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_BORDER(100, 100, 120, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SNAKE_HEAD(50, 200, 50, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SNAKE_BODY(30, 150, 30, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_FOOD(200, 50, 50, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_TEXT(255, 255, 255, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SELECTOR_BG(70, 130, 180, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SELECTED_TEXT(255, 255, 255, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_FIRE_FOOD(255, 140, 0, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_FROSTY_FOOD(80, 200, 235, 255);
const RaylibGraphics::Color RaylibGraphics::COLOR_FIRE_TILE(200, 60, 40, 255);

// Alternative palette
const RaylibGraphics::Color RaylibGraphics::ALT_COLOR_BACKGROUND(15, 15, 18, 255);
const RaylibGraphics::Color RaylibGraphics::ALT_COLOR_BORDER(180, 160, 90, 255);
const RaylibGraphics::Color RaylibGraphics::ALT_COLOR_SNAKE_HEAD(80, 180, 220, 255);
const RaylibGraphics::Color RaylibGraphics::ALT_COLOR_SNAKE_BODY(40, 120, 180, 255);
const RaylibGraphics::Color RaylibGraphics::ALT_COLOR_FOOD(235, 130, 35, 255);
const RaylibGraphics::Color RaylibGraphics::ALT_COLOR_TEXT(240, 240, 240, 255);

RaylibGraphics::RaylibGraphics()
    : _initialized(false), _shouldContinue(true), _targetFPS(60), _menuSystem(nullptr), _switchMessageTimer(0) {}

RaylibGraphics::~RaylibGraphics() {
    shutdown();
}

int RaylibGraphics::initialize() {
    if (_initialized)
        return 0;
    clearError();

    // Extended delay to ensure previous graphics context (especially SFML) is fully released
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Force a complete yield to let the system process any pending graphics cleanup
    std::this_thread::yield();

    // Additional delay specifically for OpenGL context cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Try to initialize Raylib window with error handling
    try {
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Nibbler - Raylib");

        // Give Raylib time to fully initialize its OpenGL context
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (!IsWindowReady()) {
            setError("Failed to create Raylib window - OpenGL context conflict or graphics driver issue");
            return 1;
        }

        // Additional setup to ensure clean state
        SetTargetFPS(_targetFPS);
        // Disable default ESC-to-exit so ESC behaves as back in menus
        SetExitKey(KEY_NULL);

        // Clear any potential OpenGL errors from context switch
        ClearBackground({0, 0, 0, 255}); // Use explicit Color struct
        EndDrawing(); // Force a frame to initialize graphics state
        BeginDrawing();

    } catch (...) {
        setError("Exception during Raylib window initialization - likely OpenGL context conflict");
        return 1;
    }

    _initialized = true;
    _shouldContinue = true;
    return 0;
}

void RaylibGraphics::shutdown() {
    if (!_initialized)
        return;
    _shouldContinue = false;
    if (IsWindowReady())
        CloseWindow();
    _initialized = false;
    clearError();
}

void RaylibGraphics::render(const game_data& game) {
    if (!_initialized || !IsWindowReady())
        return;

    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& bg = useAlt ? ALT_COLOR_BACKGROUND : COLOR_BACKGROUND;
    const Color& border = useAlt ? ALT_COLOR_BORDER : COLOR_BORDER;
    const Color& head = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& body = useAlt ? ALT_COLOR_SNAKE_BODY : COLOR_SNAKE_BODY;
    const Color& food = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    BeginDrawing();
    ClearBackground({bg.r, bg.g, bg.b, bg.a});

    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        renderMenu(game);
        EndDrawing();
        return;
    }

    int offsetX, offsetY, cellSize;
    calculateGameArea(game, offsetX, offsetY, cellSize);

    size_t width = game.get_width();
    size_t height = game.get_height();

    // Border (toggleable)
    bool showBorders = _menuSystem && _menuSystem->getSettings().showBorders;
    if (showBorders) {
        DrawRectangleLinesEx({(float)offsetX - 2, (float)offsetY - 2, (float)width * cellSize + 4, (float)height * cellSize + 4}, 2, {border.r, border.g, border.b, border.a});
    }

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int px = offsetX + (int)x * cellSize;
            int py = offsetY + (int)y * cellSize;
            int l2 = game.get_map_value((int)x, (int)y, 2);
            if (l2 == FOOD) {
                DrawRectangle(px, py, cellSize, cellSize, {food.r, food.g, food.b, food.a});
            } else if (l2 == FIRE_FOOD) {
                DrawRectangle(px, py, cellSize, cellSize, {COLOR_FIRE_FOOD.r, COLOR_FIRE_FOOD.g, COLOR_FIRE_FOOD.b, COLOR_FIRE_FOOD.a});
            } else if (l2 == FROSTY_FOOD) {
                DrawRectangle(px, py, cellSize, cellSize, {COLOR_FROSTY_FOOD.r, COLOR_FROSTY_FOOD.g, COLOR_FROSTY_FOOD.b, COLOR_FROSTY_FOOD.a});
            } else if (l2 >= SNAKE_HEAD_PLAYER_1 && l2 < SNAKE_HEAD_PLAYER_1 + 1000000) {
                bool headTile = (l2 % 1000000 == 1);
                auto c = headTile ? head : body;
                DrawRectangle(px, py, cellSize, cellSize, {c.r, c.g, c.b, c.a});
            } else {
                int l0 = game.get_map_value((int)x, (int)y, 0);
                if (l0 == GAME_TILE_WALL)
                    DrawRectangle(px, py, cellSize, cellSize, {border.r, border.g, border.b, border.a});
                else if (l0 == GAME_TILE_ICE)
                    DrawRectangle(px, py, cellSize, cellSize, {bg.r, bg.g, bg.b, bg.a});
                else if (l0 == GAME_TILE_FIRE)
                    DrawRectangle(px, py, cellSize, cellSize, {COLOR_FIRE_TILE.r, COLOR_FIRE_TILE.g, COLOR_FIRE_TILE.b, COLOR_FIRE_TILE.a});
            }
        }
    }

    // HUD: score/length top-left and optional FPS
    DrawText(TextFormat("Length: %d", game.get_snake_length(0)), 10, 10, 20, {text.r, text.g, text.b, text.a});
    if (_menuSystem && _menuSystem->getSettings().showFPS) {
        DrawText(TextFormat("FPS: %d", _targetFPS), 10, 34, 16, {text.r, text.g, text.b, text.a});
    }

    // Do not display any library switch message per requirements

    EndDrawing();
}

GameKey RaylibGraphics::getInput() {
    if (!_initialized || !IsWindowReady())
        return GameKey::NONE;

    if (WindowShouldClose()) {
        _shouldContinue = false;
        return GameKey::ESCAPE;
    }

    // Graphics switching
    if (IsKeyPressed(KEY_ONE))
        return GameKey::KEY_1;
    if (IsKeyPressed(KEY_TWO))
        return GameKey::KEY_2;
    if (IsKeyPressed(KEY_THREE))
        return GameKey::KEY_3;
    if (IsKeyPressed(KEY_FOUR))
        return GameKey::KEY_4;

    // Menu navigation
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        if (IsKeyPressed(KEY_UP)) {
            _menuSystem->navigateUp();
            return GameKey::NONE;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            _menuSystem->navigateDown();
            return GameKey::NONE;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            _menuSystem->selectCurrentItem();
            return GameKey::NONE;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            _menuSystem->goBack();
            return GameKey::NONE;
        }
        return GameKey::NONE;
    }

    // Game input
    if (IsKeyDown(KEY_UP))
        return GameKey::UP;
    if (IsKeyDown(KEY_DOWN))
        return GameKey::DOWN;
    if (IsKeyDown(KEY_LEFT))
        return GameKey::LEFT;
    if (IsKeyDown(KEY_RIGHT))
        return GameKey::RIGHT;

    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
        return GameKey::ESCAPE;

    return GameKey::NONE;
}

const char* RaylibGraphics::getName() const {
    return "Raylib Graphics Library";
}

bool RaylibGraphics::shouldContinue() const {
    return _shouldContinue;
}

const char* RaylibGraphics::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

void RaylibGraphics::setFrameRate(int fps) {
    _targetFPS = fps;
    if (_initialized)
        SetTargetFPS(_targetFPS);
}

void RaylibGraphics::setMenuSystem(MenuSystem* menuSystem) {
    _menuSystem = menuSystem;
}

void RaylibGraphics::setSwitchMessage(const std::string& message, int timer) {
    _switchMessage = message;
    _switchMessageTimer = timer;
}

void RaylibGraphics::setError(const std::string& msg) {
    _errorMessage = msg;
}
void RaylibGraphics::clearError() {
    _errorMessage.clear();
}

void RaylibGraphics::calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize) {
    size_t gw = game.get_width();
    size_t gh = game.get_height();
    int maxCellW = (WINDOW_WIDTH - 40) / (int)gw;
    int maxCellH = (WINDOW_HEIGHT - 100) / (int)gh;
    cellSize = std::max(8, std::min(maxCellW, maxCellH));
    int w = (int)gw * cellSize;
    int h = (int)gh * cellSize;
    offsetX = (WINDOW_WIDTH - w) / 2;
    offsetY = 50 + (WINDOW_HEIGHT - 50 - h) / 2;
}

void RaylibGraphics::drawRect(int x, int y, int w, int h, const Color& color, bool filled) {
    ::Color rl{color.r, color.g, color.b, color.a};
    if (filled)
        DrawRectangle(x, y, w, h, rl);
    else
        DrawRectangleLines(x, y, w, h, rl);
}

void RaylibGraphics::drawText(const std::string& text, int x, int y, const Color& color, int size) {
    DrawText(text.c_str(), x, y, size, {color.r, color.g, color.b, color.a});
}

void RaylibGraphics::drawCenteredText(const std::string& text, int y, const Color& color, int size) {
    int w = MeasureText(text.c_str(), size);
    DrawText(text.c_str(), (WINDOW_WIDTH - w) / 2, y, size, {color.r, color.g, color.b, color.a});
}

void RaylibGraphics::renderMenu(const game_data& game) {
    // Only draw content; frame begin/clear/end handled by render()
    switch (_menuSystem->getCurrentState()) {
    case MenuState::MAIN_MENU:
        return renderMainMenu();
    case MenuState::SETTINGS_MENU:
        return renderSettingsMenu();
    case MenuState::CREDITS_PAGE:
        return renderCreditsMenu();
    case MenuState::INSTRUCTIONS_PAGE:
        return renderInstructionsMenu();
    case MenuState::ACHIEVEMENTS_PAGE:
        return renderAchievementsMenu(game);
    case MenuState::GAME_OVER:
        return renderGameOverMenu();
    default:
        break;
    }
}

void RaylibGraphics::renderMainMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    drawCenteredText(_menuSystem->getCurrentTitle(), 100, title, 48);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 200);
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, text, 16);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, text, 16);
}

void RaylibGraphics::renderSettingsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;

    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 120);
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;
    drawCenteredText("Use Arrow Keys to navigate, ENTER to toggle/adjust", WINDOW_HEIGHT - 100, text, 16);
    drawCenteredText("ESC to go back", WINDOW_HEIGHT - 80, text, 16);
}

void RaylibGraphics::renderCreditsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);

    const auto& content = _menuSystem->getCreditsContent();
    int top = 120;
    int lineH = std::max(22, 20);
    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;
    int bottomY = WINDOW_HEIGHT - 100;

    int bonusIdx = -1;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == "BONUS FEATURES:") {
            bonusIdx = static_cast<int>(i);
            break;
        }
    }

    if (bonusIdx >= 0) {
        int y1 = top;
        for (int i = 0; i < bonusIdx && y1 <= bottomY - lineH; ++i) {
            if (content[i].empty()) {
                y1 += lineH;
                continue;
            }
            drawText(content[i], colX1, y1, text, 20);
            y1 += lineH;
        }
        int y2 = top;
        for (size_t i = bonusIdx; i < content.size() && y2 <= bottomY - lineH; ++i) {
            if (content[i].empty()) {
                y2 += lineH;
                continue;
            }
            drawText(content[i], colX2, y2, text, 20);
            y2 += lineH;
        }
    } else {
        int usableH = bottomY - top - 20;
        int linesPerCol = std::max(1, usableH / lineH);
        for (size_t i = 0; i < content.size(); ++i) {
            int col = static_cast<int>(i) / linesPerCol;
            int row = static_cast<int>(i) % linesPerCol;
            int x = (col % 2 == 0) ? colX1 : colX2;
            int y = top + row * lineH;
            if (y > bottomY)
                continue;
            drawText(content[i], x, y, text, 20);
        }
    }

    drawCenteredText("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 60, title, 18);
}

void RaylibGraphics::renderInstructionsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);

    const auto& content = _menuSystem->getInstructionsContent();
    int top = 120;
    int lineH = std::max(22, 20);
    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;
    int bottomY = WINDOW_HEIGHT - 100;

    int graphicsIdx = -1;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == "GRAPHICS LIBRARIES:") { graphicsIdx = static_cast<int>(i); break; }
    }

    if (graphicsIdx >= 0) {
        int y1 = top;
        for (int i = 0; i < graphicsIdx && y1 <= bottomY - lineH; ++i) {
            if (content[i].empty()) { y1 += lineH; continue; }
            drawText(content[i], colX1, y1, text, 20);
            y1 += lineH;
        }
        int y2 = top;
        for (size_t i = graphicsIdx; i < content.size() && y2 <= bottomY - lineH; ++i) {
            if (content[i].empty()) { y2 += lineH; continue; }
            drawText(content[i], colX2, y2, text, 20);
            y2 += lineH;
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
            drawText(content[i], x, y, text, 20);
        }
    }

    drawCenteredText("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 60, title, 18);
}

void RaylibGraphics::renderAchievementsMenu(const game_data& game) {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);

    const auto& content = _menuSystem->getAchievementsContent(game);
    int top = 120;
    int lineH = std::max(22, 20);
    int y = top;
    for (const auto& line : content) {
        if (y > WINDOW_HEIGHT - 80) break;
        drawCenteredText(line, y, text, 20);
        y += lineH;
    }

    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT - 60, title, 18);
}

void RaylibGraphics::renderGameOverMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    drawCenteredText(_menuSystem->getCurrentTitle(), 100, title, 48);

    // Draw game over message
    drawCenteredText("Your snake has collided!", 140, title, 24);

    std::string score = std::string("Final Score: ") + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredText(score, 180, text, 24);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 240);
    // Footer to match ncurses
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 100, text, 16);
    drawCenteredText("Press ESC to return to main menu", WINDOW_HEIGHT - 80, text, 16);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, text, 16);
}

void RaylibGraphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY) {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    int y = startY;
    for (int i = 0; i < (int)items.size(); ++i) {
        const auto& item = items[i];
        if (item.text.empty()) {
            y += 15;
            continue;
        }
        int textWidth = MeasureText(item.text.c_str(), 20);
        if (item.selectable && i == selectedIndex) {
            DrawRectangle((WINDOW_WIDTH - textWidth) / 2 - 10, y - 5, textWidth + 20, 30, {COLOR_SELECTOR_BG.r, COLOR_SELECTOR_BG.g, COLOR_SELECTOR_BG.b, COLOR_SELECTOR_BG.a});
        }
        DrawText(item.text.c_str(), (WINDOW_WIDTH - textWidth) / 2, y, 20, item.selectable && i == selectedIndex ? ::Color{COLOR_SELECTED_TEXT.r, COLOR_SELECTED_TEXT.g, COLOR_SELECTED_TEXT.b, COLOR_SELECTED_TEXT.a} : ::Color{text.r, text.g, text.b, text.a});
        y += 35;
    }
}

extern "C" {
IGraphicsLibrary* createGraphicsLibrary() {
    return new RaylibGraphics();
}
void destroyGraphicsLibrary(IGraphicsLibrary* lib) {
    delete lib;
}
const char* getLibraryName() {
    return "Raylib Graphics Library";
}
const char* getLibraryVersion() {
    return "1.0.0";
}
}
