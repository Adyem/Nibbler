#include "SDL2Graphics.hpp"
#include "../game_data.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// Static color definitions
const SDL2Graphics::Color SDL2Graphics::COLOR_BACKGROUND(20, 20, 30);      // Dark blue-gray
const SDL2Graphics::Color SDL2Graphics::COLOR_BORDER(100, 100, 120);       // Light gray
const SDL2Graphics::Color SDL2Graphics::COLOR_SNAKE_HEAD(50, 200, 50);     // Bright green
const SDL2Graphics::Color SDL2Graphics::COLOR_SNAKE_BODY(30, 150, 30);     // Dark green
const SDL2Graphics::Color SDL2Graphics::COLOR_FOOD(200, 50, 50);           // Red
const SDL2Graphics::Color SDL2Graphics::COLOR_TEXT(255, 255, 255);         // White

SDL2Graphics::SDL2Graphics()
    : _initialized(false), _shouldContinue(true), _targetFPS(60), _frameDelay(1000/60),
      _window(nullptr), _renderer(nullptr) {
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

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        setError(std::string("SDL could not initialize: ") + SDL_GetError());
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

    // Create renderer
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        setError(std::string("Renderer could not be created: ") + SDL_GetError());
        SDL_DestroyWindow(_window);
        _window = nullptr;
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

    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }

    if (_window) {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }

    SDL_Quit();
    _initialized = false;
}

void SDL2Graphics::render(const game_data& game) {
    if (!_initialized) {
        return;
    }

    // Clear screen with background color
    setDrawColor(COLOR_BACKGROUND);
    SDL_RenderClear(_renderer);

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
            
            case SDL_KEYDOWN:
                return translateSDLKey(event.key.keysym.sym);
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
