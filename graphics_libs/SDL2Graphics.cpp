#include "SDL2Graphics.hpp"
#include "../game_data.hpp"
#include "../MenuSystem.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Static color definitions
const SDL2Graphics::Color SDL2Graphics::COLOR_BACKGROUND(20, 20, 30);      // Dark blue-gray
const SDL2Graphics::Color SDL2Graphics::COLOR_BORDER(100, 100, 120);       // Light gray
const SDL2Graphics::Color SDL2Graphics::COLOR_SNAKE_HEAD(50, 200, 50);     // Bright green
const SDL2Graphics::Color SDL2Graphics::COLOR_SNAKE_BODY(30, 150, 30);     // Dark green
const SDL2Graphics::Color SDL2Graphics::COLOR_FOOD(200, 50, 50);           // Red
const SDL2Graphics::Color SDL2Graphics::COLOR_TEXT(255, 255, 255);         // White

// Additional colors for better UI
const SDL2Graphics::Color SDL2Graphics::COLOR_SELECTOR_BG(70, 130, 180);   // Steel blue for selector
const SDL2Graphics::Color SDL2Graphics::COLOR_SELECTED_TEXT(255, 255, 255); // White text for selected items

SDL2Graphics::SDL2Graphics()
    : _initialized(false), _shouldContinue(true), _targetFPS(60), _frameDelay(1000/60),
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
        SDL_WINDOW_SHOWN
    );

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

    // Clear screen with background color
    setDrawColor(COLOR_BACKGROUND);
    SDL_RenderClear(_renderer);

    // Check if we should render menu instead of game
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        renderMenu();
        SDL_RenderPresent(_renderer);
        return;
    }

    // Calculate game area positioning
    int offsetX, offsetY, cellSize;
    calculateGameArea(game, offsetX, offsetY, cellSize);

    size_t width = game.get_width();
    size_t height = game.get_height();

    // Draw border
    setDrawColor(COLOR_BORDER);
    drawRect(offsetX - 2, offsetY - 2, width * cellSize + 4, height * cellSize + 4, false);

    // Draw game tiles
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            int pixelX = offsetX + x * cellSize;
            int pixelY = offsetY + y * cellSize;

            // Check layer 2 first (snake and food)
            int layer2Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 2);
            if (layer2Value == FOOD) {
                setDrawColor(COLOR_FOOD);
                drawRect(pixelX + 2, pixelY + 2, cellSize - 4, cellSize - 4);
            } else if (layer2Value == SNAKE_HEAD_PLAYER_1) {
                setDrawColor(COLOR_SNAKE_HEAD);
                drawRect(pixelX, pixelY, cellSize, cellSize);
            } else if (layer2Value > SNAKE_HEAD_PLAYER_1) {
                setDrawColor(COLOR_SNAKE_BODY);
                drawRect(pixelX, pixelY, cellSize, cellSize);
            } else {
                // Check layer 0 (terrain)
                int layer0Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 0);
                if (layer0Value == GAME_TILE_WALL) {
                    setDrawColor(COLOR_BORDER);
                    drawRect(pixelX, pixelY, cellSize, cellSize);
                } else if (layer0Value == GAME_TILE_ICE) {
                    setDrawColor(COLOR_BACKGROUND);
                    drawRect(pixelX, pixelY, cellSize, cellSize);
                }
                // Empty space - no drawing needed
            }
        }
    }

    // Draw UI text (simplified for now)
    // Note: For proper text rendering, we'd need SDL_ttf, but for now we'll skip text

    // Display switch message if active
    if (_switchMessageTimer > 0) {
        // Draw a colored notification bar at the bottom
        setDrawColor(COLOR_SNAKE_HEAD);
        drawRect(10, WINDOW_HEIGHT - 40, WINDOW_WIDTH - 20, 30, true);

        // Draw border around notification
        setDrawColor(COLOR_BORDER);
        drawRect(10, WINDOW_HEIGHT - 40, WINDOW_WIDTH - 20, 30, false);

        // Decrement timer
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
                if (key == SDLK_1) return GameKey::KEY_1;
                if (key == SDLK_2) return GameKey::KEY_2;
                if (key == SDLK_3) return GameKey::KEY_3;

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
    int maxCellWidth = (WINDOW_WIDTH - 100) / gameWidth;   // Leave 100px margin
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
void SDL2Graphics::renderMenu() {
    if (!_menuSystem) return;

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
        case MenuState::GAME_OVER:
            renderGameOverMenu();
            break;
        default:
            break;
    }
}

void SDL2Graphics::renderMainMenu() {
    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 100, _fontLarge, COLOR_SNAKE_HEAD);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int startY = 250;
    drawMenuItems(items, _menuSystem->getCurrentSelection(), startY);

    // Draw footer
    drawCenteredTextWithFont("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, _fontSmall, COLOR_TEXT);
    drawCenteredTextWithFont("Press 1/2/3 to switch graphics libraries", WINDOW_HEIGHT - 60, _fontSmall, COLOR_TEXT);
}

void SDL2Graphics::renderSettingsMenu() {
    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 60, _fontLarge, COLOR_SNAKE_HEAD);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int startY = 150;
    drawMenuItems(items, _menuSystem->getCurrentSelection(), startY);

    // Draw footer
    drawCenteredTextWithFont("Use Arrow Keys to navigate, ENTER to toggle/adjust", WINDOW_HEIGHT - 100, _fontSmall, COLOR_TEXT);
    drawCenteredTextWithFont("ESC to go back", WINDOW_HEIGHT - 80, _fontSmall, COLOR_TEXT);
}

void SDL2Graphics::renderCreditsMenu() {
    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 40, _fontLarge, COLOR_SNAKE_HEAD);

    // Draw content
    auto content = _menuSystem->getCreditsContent();
    int startY = 100;

    for (size_t i = 0; i < content.size() && startY + static_cast<int>(i) * 25 < WINDOW_HEIGHT - 60; ++i) {
        const std::string& line = content[i];
        if (line.empty()) continue;

        drawCenteredTextWithFont(line, startY + static_cast<int>(i) * 25, _fontMedium, COLOR_TEXT);
    }

    // Draw footer
    drawCenteredTextWithFont("ESC to go back", WINDOW_HEIGHT - 40, _fontSmall, COLOR_TEXT);
}

void SDL2Graphics::renderInstructionsMenu() {
    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 40, _fontLarge, COLOR_SNAKE_HEAD);

    // Draw content
    auto content = _menuSystem->getInstructionsContent();
    int startY = 100;

    for (size_t i = 0; i < content.size() && startY + static_cast<int>(i) * 25 < WINDOW_HEIGHT - 60; ++i) {
        const std::string& line = content[i];
        if (line.empty()) continue;

        drawCenteredTextWithFont(line, startY + static_cast<int>(i) * 25, _fontMedium, COLOR_TEXT);
    }

    // Draw footer
    drawCenteredTextWithFont("ESC to go back", WINDOW_HEIGHT - 40, _fontSmall, COLOR_TEXT);
}

void SDL2Graphics::renderGameOverMenu() {
    // Draw title
    drawCenteredTextWithFont(_menuSystem->getCurrentTitle(), 40, _fontLarge, COLOR_SNAKE_HEAD);

    // Draw game over message
    drawCenteredTextWithFont("Your snake has collided!", 100, _fontMedium, COLOR_FOOD);

    // Draw final score
    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredTextWithFont(scoreText, 140, _fontMedium, COLOR_SNAKE_BODY);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    int selection = _menuSystem->getCurrentSelection();
    drawMenuItems(items, selection, 200);

    // Draw instructions
    drawCenteredTextWithFont("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, _fontSmall, COLOR_TEXT);
    drawCenteredTextWithFont("ESC to go back", WINDOW_HEIGHT - 60, _fontSmall, COLOR_TEXT);
}

void SDL2Graphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY) {
    for (size_t i = 0; i < items.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == selectedIndex);

        // Draw selection highlight with transparency
        if (isSelected) {
            // Draw a semi-transparent background for the selected item
            drawTransparentRect(50, startY + static_cast<int>(i) * 40 - 5, WINDOW_WIDTH - 100, 30, COLOR_SELECTOR_BG, 120);

            // Draw a subtle border around the selected item
            setDrawColor(COLOR_SELECTOR_BG);
            drawRect(50, startY + static_cast<int>(i) * 40 - 5, WINDOW_WIDTH - 100, 30, false);
        }

        // Use white text for both selected and unselected items for better readability
        Color itemColor = COLOR_SELECTED_TEXT;
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
        nullptr
    };

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
    _fontLarge = TTF_OpenFont(selectedFontPath, 32);   // For titles
    _fontMedium = TTF_OpenFont(selectedFontPath, 20);  // For menu items
    _fontSmall = TTF_OpenFont(selectedFontPath, 16);   // For instructions

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
    if (!font || text.empty()) return;

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
    if (!font || text.empty()) return;

    int textWidth = getTextWidth(text, font);
    int x = (WINDOW_WIDTH - textWidth) / 2;
    drawTextWithFont(text, x, y, font, color);
}

int SDL2Graphics::getTextWidth(const std::string& text, TTF_Font* font) {
    if (!font || text.empty()) return 0;

    int width, height;
    if (TTF_SizeText(font, text.c_str(), &width, &height) != 0) {
        return 0; // Error occurred
    }
    return width;
}

int SDL2Graphics::getTextHeight(TTF_Font* font) {
    if (!font) return 0;
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
