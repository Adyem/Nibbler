#include "SDL2Graphics.hpp"
#include "../../game_data.hpp"
#include "../../MenuSystem.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Static color definitions
const SDL2Graphics::Color SDL2Graphics::COLOR_BACKGROUND(20, 20, 30);  // Dark blue-gray
const SDL2Graphics::Color SDL2Graphics::COLOR_BORDER(100, 100, 120);   // Light gray
const SDL2Graphics::Color SDL2Graphics::COLOR_SNAKE_HEAD(50, 200, 50); // Bright green
const SDL2Graphics::Color SDL2Graphics::COLOR_SNAKE_BODY(30, 150, 30); // Dark green
const SDL2Graphics::Color SDL2Graphics::COLOR_FOOD(200, 50, 50);       // Red
const SDL2Graphics::Color SDL2Graphics::COLOR_TEXT(255, 255, 255);     // White
// Extra tiles/foods
const SDL2Graphics::Color SDL2Graphics::COLOR_FIRE_FOOD(235, 80, 35);   // Orange-red for fire food
const SDL2Graphics::Color SDL2Graphics::COLOR_FROSTY_FOOD(80, 200, 235);// Cyan for frosty food
const SDL2Graphics::Color SDL2Graphics::COLOR_FIRE_TILE(200, 60, 40);    // Red for fire tile

// Alternative palette
const SDL2Graphics::Color SDL2Graphics::ALT_COLOR_BACKGROUND(15, 15, 18);  // Darker
const SDL2Graphics::Color SDL2Graphics::ALT_COLOR_BORDER(180, 160, 90);    // Sand/gold
const SDL2Graphics::Color SDL2Graphics::ALT_COLOR_SNAKE_HEAD(80, 180, 220); // Cyan head
const SDL2Graphics::Color SDL2Graphics::ALT_COLOR_SNAKE_BODY(40, 120, 180); // Blue body
const SDL2Graphics::Color SDL2Graphics::ALT_COLOR_FOOD(235, 130, 35);      // Orange
const SDL2Graphics::Color SDL2Graphics::ALT_COLOR_TEXT(240, 240, 240);     // Near white

// Additional colors for better UI
const SDL2Graphics::Color SDL2Graphics::COLOR_SELECTOR_BG(70, 130, 180);    // Steel blue for selector
const SDL2Graphics::Color SDL2Graphics::COLOR_SELECTED_TEXT(255, 255, 255); // White text for selected items

SDL2Graphics::SDL2Graphics()
    : _initialized(false), _shouldContinue(true), _targetFPS(60), _frameDelay(1000 / 60),
      _window(nullptr), _renderer(nullptr), _fontLarge(nullptr), _fontMedium(nullptr),
      _fontSmall(nullptr), _menuSystem(nullptr), _switchMessageTimer(0) {
}

SDL2Graphics::~SDL2Graphics() {
    if (_initialized) {
        shutdown();
    }
}

int SDL2Graphics::initialize() {
    if (_initialized) {
        return 0;
    }

    clearError();

    // Initialize SDL - always try to create a real window
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        setError(std::string("SDL could not initialize: ") + SDL_GetError());
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        setError(std::string("SDL_ttf could not initialize: ") + TTF_GetError());
        SDL_Quit();
        return 1;
    }

    // Create window
    _window = SDL_CreateWindow(
        "Nibbler - SDL2 Graphics",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);

    if (!_window) {
        setError(std::string("Window could not be created: ") + SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer - try hardware acceleration first, then software
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        // If hardware acceleration failed, try software rendering
        _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);

        if (!_renderer) {
            setError(std::string("Renderer could not be created: ") + SDL_GetError());
            SDL_DestroyWindow(_window);
            _window = nullptr;
            TTF_Quit();
            SDL_Quit();
            return 1;
        }
    }

    // Initialize fonts
    if (!initializeFonts()) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
        SDL_DestroyWindow(_window);
        _window = nullptr;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    _initialized = true;
    return 0;
}

void SDL2Graphics::shutdown() {
    if (!_initialized) {
        return;
    }

    // Hide the window immediately to make transition feel smoother
    if (_window) {
        SDL_HideWindow(_window);
        // Force window to update its state
        SDL_PumpEvents();

        // Try to minimize the window to help with focus transfer
        SDL_MinimizeWindow(_window);
        SDL_PumpEvents();
    }

    // Process any remaining events to ensure clean shutdown
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Just consume the events, don't process them
    }

    // Shutdown fonts first
    shutdownFonts();

    // Destroy renderer
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }

    // Destroy window and force it to close immediately
    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    // Process events one more time after window destruction
    while (SDL_PollEvent(&event)) {
        // Consume any remaining events
    }

    // Force all events to be processed
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

    // Quit SDL subsystems
    TTF_Quit();
    SDL_Quit();

    // Small delay to ensure window is fully closed
    SDL_Delay(50);

    _initialized = false;
}

void SDL2Graphics::render(const game_data& game) {
    if (!_initialized) {
        return;
    }

    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& bg = useAlt ? ALT_COLOR_BACKGROUND : COLOR_BACKGROUND;
    const Color& border = useAlt ? ALT_COLOR_BORDER : COLOR_BORDER;
    const Color& head = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& body = useAlt ? ALT_COLOR_SNAKE_BODY : COLOR_SNAKE_BODY;
    const Color& food = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Clear screen with background color
    setDrawColor(bg);
    SDL_RenderClear(_renderer);

    // Check if we should render menu instead of game
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        renderMenu(game);
        SDL_RenderPresent(_renderer);
        return;
    }

    // Calculate game area positioning
    int offsetX, offsetY, cellSize;
    calculateGameArea(game, offsetX, offsetY, cellSize);

    size_t width = game.get_width();
    size_t height = game.get_height();

    // Draw border (toggleable)
    bool showBorders = _menuSystem && _menuSystem->getSettings().showBorders;
    if (showBorders) {
        setDrawColor(border);
        drawRect(offsetX - 2, offsetY - 2, width * cellSize + 4, height * cellSize + 4, false);
    }

    // Draw game tiles
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int pixelX = offsetX + x * cellSize;
            int pixelY = offsetY + y * cellSize;

            // Check layer 2 first (snake and food)
            int layer2Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 2);
            if (layer2Value == FOOD) {
                setDrawColor(food);
                drawRect(pixelX + 2, pixelY + 2, cellSize - 4, cellSize - 4);
            } else if (layer2Value == FIRE_FOOD) {
                setDrawColor(COLOR_FIRE_FOOD);
                drawRect(pixelX + 2, pixelY + 2, cellSize - 4, cellSize - 4);
            } else if (layer2Value == FROSTY_FOOD) {
                setDrawColor(COLOR_FROSTY_FOOD);
                drawRect(pixelX + 2, pixelY + 2, cellSize - 4, cellSize - 4);
            } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
                setDrawColor(head);
                drawRect(pixelX, pixelY, cellSize, cellSize);
            } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
                setDrawColor(body);
                drawRect(pixelX, pixelY, cellSize, cellSize);
            } else {
                // Check layer 0 (terrain)
                int layer0Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 0);
                if (layer0Value == GAME_TILE_WALL) {
                    setDrawColor(border);
                    drawRect(pixelX, pixelY, cellSize, cellSize);
                } else if (layer0Value == GAME_TILE_ICE) {
                    setDrawColor(bg);
                    drawRect(pixelX, pixelY, cellSize, cellSize);
                } else if (layer0Value == GAME_TILE_FIRE) {
                    setDrawColor(COLOR_FIRE_TILE);
                    drawRect(pixelX, pixelY, cellSize, cellSize);
                }
                // Empty space - no drawing needed
            }
        }
    }

    // HUD: show snake length and optional FPS in top-left
    {
        std::string scoreText = "Length: " + std::to_string(game.get_snake_length(0));
        drawTextWithFont(scoreText, 10, 10, _fontMedium, text);
        if (_menuSystem && _menuSystem->getSettings().showFPS) {
            drawTextWithFont(std::string("FPS: ") + std::to_string(_targetFPS), 10, 35, _fontSmall, text);
        }
    }

    // Remove green switch bar; only decrement timer silently
    if (_switchMessageTimer > 0) {
        _switchMessageTimer--;
    }

    // Present the rendered frame
    SDL_RenderPresent(_renderer);
}

GameKey SDL2Graphics::getInput() {
    if (!_initialized) {
        return GameKey::NONE;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            _shouldContinue = false;
            return GameKey::QUIT;

        case SDL_KEYDOWN: {
            SDL_Keycode key = event.key.keysym.sym;

            // Handle graphics switching first (works in any state)
            if (key == SDLK_1)
                return GameKey::KEY_1;
            if (key == SDLK_2)
                return GameKey::KEY_2;
            if (key == SDLK_3)
                return GameKey::KEY_3;
            if (key == SDLK_4)
                return GameKey::KEY_4;

            // Handle menu navigation if in menu mode
            if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
                switch (key) {
                case SDLK_UP:
                    _menuSystem->navigateUp();
                    return GameKey::NONE;
                case SDLK_DOWN:
                    _menuSystem->navigateDown();
                    return GameKey::NONE;
                case SDLK_RETURN:
                case SDLK_SPACE:
                    _menuSystem->selectCurrentItem();
                    return GameKey::NONE;
                case SDLK_ESCAPE:
                    _menuSystem->goBack();
                    return GameKey::NONE;
                default:
                    return GameKey::NONE;
                }
            }

            // In game mode, translate all keys normally
            return translateSDLKey(key);
        }
        }
    }

    return GameKey::NONE;
}

const char* SDL2Graphics::getName() const {
    return "SDL2 Graphics";
}

bool SDL2Graphics::shouldContinue() const {
    return _shouldContinue;
}

const char* SDL2Graphics::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

void SDL2Graphics::setFrameRate(int fps) {
    _targetFPS = fps;
    _frameDelay = (fps > 0) ? (1000 / fps) : (1000 / 60);
}

void SDL2Graphics::setMenuSystem(MenuSystem* menuSystem) {
    _menuSystem = menuSystem;
}

// Private helper methods
void SDL2Graphics::setError(const std::string& error) {
    _errorMessage = error;
}

void SDL2Graphics::clearError() {
    _errorMessage.clear();
}

void SDL2Graphics::setDrawColor(const Color& color) {
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
}

void SDL2Graphics::drawRect(int x, int y, int width, int height, bool filled) {
    SDL_Rect rect = {x, y, width, height};
    if (filled) {
        SDL_RenderFillRect(_renderer, &rect);
    } else {
        SDL_RenderDrawRect(_renderer, &rect);
    }
}

void SDL2Graphics::drawTransparentRect(int x, int y, int width, int height, const Color& color, Uint8 alpha) {
    // Set blend mode for transparency
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

    // Set color with alpha
    SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, alpha);

    // Draw filled rectangle
    SDL_Rect rect = {x, y, width, height};
    SDL_RenderFillRect(_renderer, &rect);

    // Reset blend mode to default
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
}

GameKey SDL2Graphics::translateSDLKey(SDL_Keycode key) {
    switch (key) {
    case SDLK_UP:
        return GameKey::UP;
    case SDLK_DOWN:
        return GameKey::DOWN;
    case SDLK_LEFT:
        return GameKey::LEFT;
    case SDLK_RIGHT:
        return GameKey::RIGHT;
    case SDLK_1:
        return GameKey::KEY_1;
    case SDLK_2:
        return GameKey::KEY_2;
    case SDLK_3:
        return GameKey::KEY_3;
    case SDLK_ESCAPE:
    case SDLK_q:
        return GameKey::ESCAPE;
    default:
        return GameKey::NONE;
    }
}

void SDL2Graphics::calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize) {
    size_t gameWidth = game.get_width();
    size_t gameHeight = game.get_height();

    // Calculate the best cell size that fits in the window
    int maxCellWidth = (WINDOW_WIDTH - 100) / gameWidth;    // Leave 100px margin
    int maxCellHeight = (WINDOW_HEIGHT - 150) / gameHeight; // Leave 150px for UI

    cellSize = std::min(maxCellWidth, maxCellHeight);
    cellSize = std::max(cellSize, 10); // Minimum cell size

    // Center the game area
    int totalGameWidth = gameWidth * cellSize;
    int totalGameHeight = gameHeight * cellSize;

    offsetX = (WINDOW_WIDTH - totalGameWidth) / 2;
    offsetY = (WINDOW_HEIGHT - totalGameHeight) / 2;
}

void SDL2Graphics::setSwitchMessage(const std::string& message, int timer) {
    _switchMessage = message;
    _switchMessageTimer = timer;
}

// Menu rendering methods
void SDL2Graphics::renderMenu(const game_data& game) {
    if (!_menuSystem)
        return;

    bool useAlt = _menuSystem->getSettings().useAlternativeColors;
    const Color& bg = useAlt ? ALT_COLOR_BACKGROUND : COLOR_BACKGROUND;

    // Background
    setDrawColor(bg);
    SDL_RenderClear(_renderer);

    switch (_menuSystem->getCurrentState()) {
        case MenuState::MAIN_MENU:
            renderMainMenu();
            break;
        case MenuState::SETTINGS_MENU:
            renderSettingsMenu();
            break;
        case MenuState::CREDITS_PAGE:
            renderCreditsMenu();
            break;
        case MenuState::INSTRUCTIONS_PAGE:
            renderInstructionsMenu();
            break;
        case MenuState::ACHIEVEMENTS_PAGE:
            renderAchievementsMenu(game);
            break;
        case MenuState::GAME_OVER:
            renderGameOverMenu();
            break;
        default:
            break;
    }
}

void SDL2Graphics::renderMainMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 100, _fontLarge, title);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int startY = 250;
    drawMenuItems(items, _menuSystem->getCurrentSelection(), startY);

    // Draw footer
    drawCenteredTextWithFont("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, _fontSmall, text);
    drawCenteredTextWithFont("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, _fontSmall, text);
}

void SDL2Graphics::renderSettingsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 60, _fontLarge, title);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int startY = 150;
    drawMenuItems(items, _menuSystem->getCurrentSelection(), startY);

    // Draw footer
    drawCenteredTextWithFont("Use Arrow Keys to navigate, ENTER to toggle/adjust", WINDOW_HEIGHT - 100, _fontSmall, text);
    drawCenteredTextWithFont("ESC to go back", WINDOW_HEIGHT - 80, _fontSmall, text);
}

void SDL2Graphics::renderCreditsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title (smaller to fit more content)
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 40, _fontMedium, title);

    // Two-column content with "BONUS FEATURES:" forced to second column
    const auto& content = _menuSystem->getCreditsContent();
    int top = 100;
    // Smaller line height to fit more lines
    int lineH = std::max(18, getTextHeight(_fontSmall) + 2);
    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;
    int bottomY = WINDOW_HEIGHT - 60;

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
            drawTextWithFont(content[i], colX1, y1, _fontSmall, text);
            y1 += lineH;
        }
        int y2 = top;
        for (size_t i = bonusIdx; i < content.size() && y2 <= bottomY - lineH; ++i) {
            if (content[i].empty()) {
                y2 += lineH;
                continue;
            }
            drawTextWithFont(content[i], colX2, y2, _fontSmall, text);
            y2 += lineH;
        }
    } else {
        // Fallback: auto two-column flow
        int usableH = bottomY - top - 20;
        int linesPerCol = std::max(1, usableH / lineH);
        for (size_t i = 0; i < content.size(); ++i) {
            int col = static_cast<int>(i) / linesPerCol;
            int row = static_cast<int>(i) % linesPerCol;
            int x = (col % 2 == 0) ? colX1 : colX2;
            int y = top + row * lineH;
            if (y > bottomY)
                continue;
            drawTextWithFont(content[i], x, y, _fontSmall, text);
        }
    }

    // Footer
    drawCenteredTextWithFont("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 40, _fontSmall, text);
}

void SDL2Graphics::renderInstructionsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 40, _fontLarge, title);

    // Two-column content with second column starting at "GRAPHICS LIBRARIES:"
    const auto& content = _menuSystem->getInstructionsContent();
    int top = 100;
    int lineH = std::max(22, getTextHeight(_fontMedium) + 4);
    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;
    int bottomY = WINDOW_HEIGHT - 60;

    int graphicsIdx = -1;
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == "GRAPHICS LIBRARIES:") {
            graphicsIdx = static_cast<int>(i);
            break;
        }
    }

    if (graphicsIdx >= 0) {
        int y1 = top;
        for (int i = 0; i < graphicsIdx && y1 <= bottomY - lineH; ++i) {
            if (content[i].empty()) { y1 += lineH; continue; }
            drawTextWithFont(content[i], colX1, y1, _fontMedium, text);
            y1 += lineH;
        }
        int y2 = top;
        for (size_t i = graphicsIdx; i < content.size() && y2 <= bottomY - lineH; ++i) {
            if (content[i].empty()) { y2 += lineH; continue; }
            drawTextWithFont(content[i], colX2, y2, _fontMedium, text);
            y2 += lineH;
        }
    } else {
        // Fallback: auto flow
        int usableH = bottomY - top - 20;
        int linesPerCol = std::max(1, usableH / lineH);
        for (size_t i = 0; i < content.size(); ++i) {
            int col = static_cast<int>(i) / linesPerCol;
            int row = static_cast<int>(i) % linesPerCol;
            int x = (col % 2 == 0) ? colX1 : colX2;
            int y = top + row * lineH;
            if (y > bottomY) continue;
            drawTextWithFont(content[i], x, y, _fontMedium, text);
        }
    }

    // Footer
    drawCenteredTextWithFont("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 40, _fontSmall, text);
}

void SDL2Graphics::renderGameOverMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;
    const Color& secondary = useAlt ? ALT_COLOR_SNAKE_BODY : COLOR_SNAKE_BODY;

    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 40, _fontLarge, title);

    // Draw game over message
    drawCenteredTextWithFont("Your snake has collided!", 100, _fontMedium, title);

    // Draw final score
    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredTextWithFont(scoreText, 140, _fontMedium, secondary);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int selection = _menuSystem->getCurrentSelection();
    drawMenuItems(items, selection, 200);

    // Draw instructions
    drawCenteredTextWithFont("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 100, _fontSmall, text);
    drawCenteredTextWithFont("Press ESC to return to main menu", WINDOW_HEIGHT - 80, _fontSmall, text);
    drawCenteredTextWithFont("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, _fontSmall, text);
}

void SDL2Graphics::renderAchievementsMenu(const game_data& game) {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 60, _fontLarge, title);

    // Content lines
    const auto& content = _menuSystem->getAchievementsContent(game);
    int top = 120;
    int lineH = std::max(22, getTextHeight(_fontMedium) + 4);
    int y = top;
    for (const auto& line : content) {
        if (y > WINDOW_HEIGHT - 80) break;
        drawCenteredTextWithFont(line, y, _fontMedium, text);
        y += lineH;
    }

    // Footer
    drawCenteredTextWithFont("Press ESC or ENTER to return to main menu", WINDOW_HEIGHT - 50, _fontSmall, text);
}

void SDL2Graphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY) {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_SELECTED_TEXT; // selected uses white
    const Color& selector = COLOR_SELECTOR_BG; // keep same for both palettes for contrast

    for (size_t i = 0; i < items.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == selectedIndex);

        // Draw selection highlight with transparency
        if (isSelected) {
            // Draw a semi-transparent background for the selected item
            drawTransparentRect(50, startY + static_cast<int>(i) * 40 - 5, WINDOW_WIDTH - 100, 30, selector, 120);

            // Draw a subtle border around the selected item
            setDrawColor(selector);
            drawRect(50, startY + static_cast<int>(i) * 40 - 5, WINDOW_WIDTH - 100, 30, false);
        }

        // Use white text for selected and palette text for unselected
        Color itemColor = isSelected ? COLOR_SELECTED_TEXT : text;
        drawCenteredTextWithFont(items[i].text, startY + static_cast<int>(i) * 40, _fontMedium, itemColor);
    }
}

// Font implementation methods
bool SDL2Graphics::initializeFonts() {
    // Try to load Arial font from common system locations
    const char* fontPaths[] = {
        // macOS - Arial fonts
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
        "/System/Library/Fonts/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        // macOS - Helvetica as fallback
        "/System/Library/Fonts/HelveticaNeue.ttc",
        "/System/Library/Fonts/Geneva.ttf",
        // Windows
        "C:/Windows/Fonts/arial.ttf",
        // Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/arial.ttf",
        // Fallback - try to find any reasonable font
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        nullptr};

    TTF_Font* testFont = nullptr;
    const char* selectedFontPath = nullptr;

    // Find the first available font
    for (int i = 0; fontPaths[i] != nullptr; ++i) {
        testFont = TTF_OpenFont(fontPaths[i], 24);
        if (testFont) {
            selectedFontPath = fontPaths[i];
            TTF_CloseFont(testFont);
            break;
        }
    }

    if (!selectedFontPath) {
        setError("Could not find any suitable font file. Please install Arial or DejaVu Sans fonts.");
        return false;
    }

    // Load fonts in different sizes
    _fontLarge = TTF_OpenFont(selectedFontPath, 32);  // For titles
    _fontMedium = TTF_OpenFont(selectedFontPath, 20); // For menu items
    _fontSmall = TTF_OpenFont(selectedFontPath, 16);  // For instructions

    if (!_fontLarge || !_fontMedium || !_fontSmall) {
        setError(std::string("Failed to load font: ") + TTF_GetError());
        shutdownFonts();
        return false;
    }

    return true;
}

void SDL2Graphics::shutdownFonts() {
    if (_fontLarge) {
        TTF_CloseFont(_fontLarge);
        _fontLarge = nullptr;
    }
    if (_fontMedium) {
        TTF_CloseFont(_fontMedium);
        _fontMedium = nullptr;
    }
    if (_fontSmall) {
        TTF_CloseFont(_fontSmall);
        _fontSmall = nullptr;
    }
}

void SDL2Graphics::drawTextWithFont(const std::string& text, int x, int y, TTF_Font* font, const Color& color) {
    if (!font || text.empty())
        return;

    // Clean the text string to handle problematic ASCII characters
    std::string cleanText = text;

    // Replace or filter out problematic characters
    for (size_t i = 0; i < cleanText.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(cleanText[i]);

        // Replace non-printable ASCII characters (except common whitespace)
        if (c < 32 && c != '\t' && c != '\n' && c != '\r') {
            cleanText[i] = ' '; // Replace with space
        }
        // Handle extended ASCII characters that might cause issues
        else if (c > 126 && c < 160) {
            cleanText[i] = '?'; // Replace with question mark
        }
    }

    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};

    // Use blended rendering for better quality and anti-aliasing
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, cleanText.c_str(), sdlColor);

    if (!textSurface) {
        // Fallback to solid rendering if blended fails
        textSurface = TTF_RenderText_Solid(font, cleanText.c_str(), sdlColor);
        if (!textSurface) {
            return; // Failed to render text
        }
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(_renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (!textTexture) {
        return; // Failed to create texture
    }

    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textWidth, &textHeight);

    SDL_Rect destRect = {x, y, textWidth, textHeight};
    SDL_RenderCopy(_renderer, textTexture, nullptr, &destRect);

    SDL_DestroyTexture(textTexture);
}

void SDL2Graphics::drawCenteredTextWithFont(const std::string& text, int y, TTF_Font* font, const Color& color) {
    if (!font || text.empty())
        return;

    int textWidth = getTextWidth(text, font);
    int x = (WINDOW_WIDTH - textWidth) / 2;
    drawTextWithFont(text, x, y, font, color);
}

int SDL2Graphics::getTextWidth(const std::string& text, TTF_Font* font) {
    if (!font || text.empty())
        return 0;

    int width, height;
    if (TTF_SizeText(font, text.c_str(), &width, &height) != 0) {
        return 0; // Error occurred
    }
    return width;
}

int SDL2Graphics::getTextHeight(TTF_Font* font) {
    if (!font)
        return 0;
    return TTF_FontHeight(font);
}

// C interface for dynamic library loading
extern "C" {
IGraphicsLibrary* createGraphicsLibrary() {
    return new SDL2Graphics();
}

void destroyGraphicsLibrary(IGraphicsLibrary* lib) {
    delete lib;
}

const char* getLibraryName() {
    return "SDL2 Graphics Library";
}

const char* getLibraryVersion() {
    return "1.0.0";
}
}
