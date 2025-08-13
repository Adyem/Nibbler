#include "RaylibGraphics.hpp"
#include "../game_data.hpp"
#include "../MenuSystem.hpp"
#include <raylib.h>
#include <algorithm>

const RaylibGraphics::Color RaylibGraphics::COLOR_BACKGROUND(20,20,30,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_BORDER(100,100,120,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SNAKE_HEAD(50,200,50,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SNAKE_BODY(30,150,30,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_FOOD(200,50,50,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_TEXT(255,255,255,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SELECTOR_BG(70,130,180,255);
const RaylibGraphics::Color RaylibGraphics::COLOR_SELECTED_TEXT(255,255,255,255);

RaylibGraphics::RaylibGraphics()
: _initialized(false), _shouldContinue(true), _targetFPS(60), _menuSystem(nullptr), _switchMessageTimer(0) {}

RaylibGraphics::~RaylibGraphics(){ shutdown(); }

int RaylibGraphics::initialize(){
    if (_initialized) return 0;
    clearError();

    // raylib is happy from a plugin on macOS
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Nibbler - Raylib");
    if (!IsWindowReady()) { setError("Failed to create window"); return 1; }
    SetTargetFPS(_targetFPS);
    _initialized = true;
    _shouldContinue = true;
    return 0;
}

void RaylibGraphics::shutdown(){
    if (!_initialized) return;
    _shouldContinue = false;
    if (IsWindowReady()) CloseWindow();
    _initialized = false;
    clearError();
}

void RaylibGraphics::render(const game_data& game){
    if (!_initialized || !IsWindowReady()) return;
    BeginDrawing();
    ClearBackground({COLOR_BACKGROUND.r,COLOR_BACKGROUND.g,COLOR_BACKGROUND.b,COLOR_BACKGROUND.a});

    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        renderMenu();
        EndDrawing();
        return;
    }

    int offsetX, offsetY, cellSize;
    calculateGameArea(game, offsetX, offsetY, cellSize);

    size_t width = game.get_width();
    size_t height = game.get_height();

    // Border
    DrawRectangleLinesEx({(float)offsetX-2,(float)offsetY-2,(float)width*cellSize+4,(float)height*cellSize+4}, 2, {COLOR_BORDER.r,COLOR_BORDER.g,COLOR_BORDER.b,COLOR_BORDER.a});

    for (size_t y=0; y<height; ++y) {
        for (size_t x=0; x<width; ++x) {
            int px = offsetX + (int)x * cellSize;
            int py = offsetY + (int)y * cellSize;
            int l2 = game.get_map_value((int)x,(int)y,2);
            if (l2 == FOOD) {
                DrawRectangle(px, py, cellSize, cellSize, {COLOR_FOOD.r,COLOR_FOOD.g,COLOR_FOOD.b,COLOR_FOOD.a});
            } else if (l2 >= SNAKE_HEAD_PLAYER_1 && l2 < SNAKE_HEAD_PLAYER_1 + 1000000) {
                bool head = (l2 % 1000000 == 1);
                auto c = head ? COLOR_SNAKE_HEAD : COLOR_SNAKE_BODY;
                DrawRectangle(px, py, cellSize, cellSize, {c.r,c.g,c.b,c.a});
            } else {
                int l0 = game.get_map_value((int)x,(int)y,0);
                if (l0 == GAME_TILE_WALL) DrawRectangle(px, py, cellSize, cellSize, {COLOR_BORDER.r,COLOR_BORDER.g,COLOR_BORDER.b,COLOR_BORDER.a});
                else if (l0 == GAME_TILE_ICE) DrawRectangle(px, py, cellSize, cellSize, {COLOR_BACKGROUND.r,COLOR_BACKGROUND.g,COLOR_BACKGROUND.b,COLOR_BACKGROUND.a});
            }
        }
    }

    // Score
    DrawText(TextFormat("Length: %d", game.get_snake_length(0)), 10, 10, 20, {COLOR_TEXT.r,COLOR_TEXT.g,COLOR_TEXT.b,COLOR_TEXT.a});

    EndDrawing();
}

GameKey RaylibGraphics::getInput(){
    if (!_initialized || !IsWindowReady()) return GameKey::NONE;

    if (WindowShouldClose()) { _shouldContinue = false; return GameKey::QUIT; }

    // Graphics switching
    if (IsKeyPressed(KEY_ONE)) return GameKey::KEY_1;
    if (IsKeyPressed(KEY_TWO)) return GameKey::KEY_2;
    if (IsKeyPressed(KEY_THREE)) return GameKey::KEY_3;
    if (IsKeyPressed(KEY_FOUR)) return GameKey::KEY_4;

    // Menu navigation
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        if (IsKeyPressed(KEY_UP)) { _menuSystem->navigateUp(); return GameKey::NONE; }
        if (IsKeyPressed(KEY_DOWN)) { _menuSystem->navigateDown(); return GameKey::NONE; }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { _menuSystem->selectCurrentItem(); return GameKey::NONE; }
        if (IsKeyPressed(KEY_ESCAPE)) { _menuSystem->goBack(); return GameKey::NONE; }
    }

    // Game input
    if (IsKeyPressed(KEY_UP)) return GameKey::UP;
    if (IsKeyPressed(KEY_DOWN)) return GameKey::DOWN;
    if (IsKeyPressed(KEY_LEFT)) return GameKey::LEFT;
    if (IsKeyPressed(KEY_RIGHT)) return GameKey::RIGHT;
    if (IsKeyPressed(KEY_ESCAPE)) return GameKey::ESCAPE;

    return GameKey::NONE;
}

const char* RaylibGraphics::getName() const { return "Raylib Graphics Library"; }

bool RaylibGraphics::shouldContinue() const { return _shouldContinue; }

const char* RaylibGraphics::getError() const { return _errorMessage.empty()? nullptr : _errorMessage.c_str(); }

void RaylibGraphics::setFrameRate(int fps){ _targetFPS = fps; if (_initialized) SetTargetFPS(_targetFPS); }

void RaylibGraphics::setMenuSystem(MenuSystem* menu){ _menuSystem = menu; }

void RaylibGraphics::setSwitchMessage(const std::string& message, int timer){ _switchMessage = message; _switchMessageTimer = timer; }

void RaylibGraphics::setError(const std::string& msg){ _errorMessage = msg; }
void RaylibGraphics::clearError(){ _errorMessage.clear(); }

void RaylibGraphics::calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize){
    size_t gw = game.get_width(); size_t gh = game.get_height();
    int maxCellW = (WINDOW_WIDTH - 40) / (int)gw;
    int maxCellH = (WINDOW_HEIGHT - 100) / (int)gh;
    cellSize = std::max(8, std::min(maxCellW, maxCellH));
    int w = (int)gw * cellSize;
    int h = (int)gh * cellSize;
    offsetX = (WINDOW_WIDTH - w) / 2;
    offsetY = 50 + (WINDOW_HEIGHT - 50 - h) / 2;
}

void RaylibGraphics::drawRect(int x, int y, int w, int h, const Color& color, bool filled){
    Color rl = color;
    if (filled) DrawRectangle(x,y,w,h, {rl.r,rl.g,rl.b,rl.a});
    else DrawRectangleLines(x,y,w,h, {rl.r,rl.g,rl.b,rl.a});
}

void RaylibGraphics::drawText(const std::string& text, int x, int y, const Color& color, int size){
    DrawText(text.c_str(), x, y, size, {color.r,color.g,color.b,color.a});
}

void RaylibGraphics::drawCenteredText(const std::string& text, int y, const Color& color, int size){
    int w = MeasureText(text.c_str(), size);
    DrawText(text.c_str(), (WINDOW_WIDTH - w)/2, y, size, {color.r,color.g,color.b,color.a});
}

void RaylibGraphics::renderMenu(){
    switch (_menuSystem->getCurrentState()) {
        case MenuState::MAIN_MENU: return renderMainMenu();
        case MenuState::SETTINGS_MENU: return renderSettingsMenu();
        case MenuState::CREDITS_PAGE: return renderCreditsMenu();
        case MenuState::INSTRUCTIONS_PAGE: return renderInstructionsMenu();
        case MenuState::GAME_OVER: return renderGameOverMenu();
        default: break;
    }
}

void RaylibGraphics::renderMainMenu(){
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, COLOR_SNAKE_HEAD, 48);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 200);
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT-80, COLOR_TEXT, 16);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT-60, COLOR_TEXT, 16);
}

void RaylibGraphics::renderSettingsMenu(){
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_SNAKE_HEAD, 36);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 120);
}

void RaylibGraphics::renderCreditsMenu(){
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_SNAKE_HEAD, 36);
    const auto& content = _menuSystem->getCreditsContent();
    int y=120; for (const auto& line: content){ if(!line.empty()) drawCenteredText(line, y, COLOR_TEXT, 16); y+=25; if (y>WINDOW_HEIGHT-100) break; }
    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT-60, COLOR_SNAKE_HEAD, 18);
}

void RaylibGraphics::renderInstructionsMenu(){
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_SNAKE_HEAD, 36);
    const auto& content = _menuSystem->getInstructionsContent();
    int y=120; for (const auto& line: content){ if(!line.empty()) drawCenteredText(line, y, COLOR_TEXT, 16); y+=25; if (y>WINDOW_HEIGHT-100) break; }
    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT-60, COLOR_SNAKE_HEAD, 18);
}

void RaylibGraphics::renderGameOverMenu(){
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, COLOR_FOOD, 48);
    std::string score = std::string("Final Score: ") + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredText(score, 180, COLOR_TEXT, 24);
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 240);
}

void RaylibGraphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY){
    int y=startY;
    for (int i=0;i<(int)items.size();++i){
        const auto& item = items[i];
        if (item.text.empty()) { y+=15; continue; }
        int textWidth = MeasureText(item.text.c_str(), 20);
        if (item.selectable && i==selectedIndex){
            DrawRectangle((WINDOW_WIDTH-textWidth)/2 - 10, y-5, textWidth+20, 30, {COLOR_SELECTOR_BG.r,COLOR_SELECTOR_BG.g,COLOR_SELECTOR_BG.b,COLOR_SELECTOR_BG.a});
        }
        DrawText(item.text.c_str(), (WINDOW_WIDTH-textWidth)/2, y, 20, item.selectable && i==selectedIndex ? ::Color{COLOR_SELECTED_TEXT.r,COLOR_SELECTED_TEXT.g,COLOR_SELECTED_TEXT.b,COLOR_SELECTED_TEXT.a} : ::Color{COLOR_TEXT.r,COLOR_TEXT.g,COLOR_TEXT.b,COLOR_TEXT.a});
        y += 35;
    }
}

extern "C" {
    IGraphicsLibrary* createGraphicsLibrary(){ return new RaylibGraphics(); }
    void destroyGraphicsLibrary(IGraphicsLibrary* lib){ delete lib; }
    const char* getLibraryName(){ return "Raylib Graphics Library"; }
    const char* getLibraryVersion(){ return "1.0.0"; }
}
