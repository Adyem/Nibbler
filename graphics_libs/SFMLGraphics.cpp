#include "SFMLGraphics.hpp"
#include "../game_data.hpp"
#include "../MenuSystem.hpp"
#include <SFML/System/Sleep.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

// Static color definitions
const SFMLGraphics::Color SFMLGraphics::COLOR_BACKGROUND(20, 20, 30);      // Dark blue-gray
const SFMLGraphics::Color SFMLGraphics::COLOR_BORDER(100, 100, 120);       // Light gray
const SFMLGraphics::Color SFMLGraphics::COLOR_SNAKE_HEAD(50, 200, 50);     // Bright green
const SFMLGraphics::Color SFMLGraphics::COLOR_SNAKE_BODY(30, 150, 30);     // Dark green
const SFMLGraphics::Color SFMLGraphics::COLOR_FOOD(200, 50, 50);           // Red
const SFMLGraphics::Color SFMLGraphics::COLOR_TEXT(255, 255, 255);         // White

// Additional colors for better UI
const SFMLGraphics::Color SFMLGraphics::COLOR_SELECTOR_BG(70, 130, 180);   // Steel blue for selector
const SFMLGraphics::Color SFMLGraphics::COLOR_SELECTED_TEXT(255, 255, 255); // White text for selected items

SFMLGraphics::SFMLGraphics()
    : _initialized(false), _shouldContinue(true), _targetFPS(60),
      _window(nullptr), _menuSystem(nullptr), _switchMessageTimer(0) {
}

SFMLGraphics::~SFMLGraphics() {
    shutdown();
}

int SFMLGraphics::initialize() {
    if (_initialized) {
        return 0;
    }

    clearError();

    // Create SFML window - SFML 3.x uses Vector2u for VideoMode
    _window = new sf::RenderWindow(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Nibbler - SFML Graphics");
    if (!_window) {
        setError("Failed to create SFML window");
        return 1;
    }

    // Set frame rate limit
    _window->setFramerateLimit(_targetFPS);

    // Load font
    if (!initializeFont()) {
        delete _window;
        _window = nullptr;
        return 1;
    }

    // Ensure the instance can continue after a re-initialize following a shutdown
    _shouldContinue = true;

    _initialized = true;
    return 0;
}

void SFMLGraphics::shutdown() {
    if (!_initialized) {
        return;
    }

    // Immediately mark as not continuing to stop any render loops
    _shouldContinue = false;

    if (_window) {
        // First, stop any rendering by making window invisible
        _window->setVisible(false);
        
        // Aggressively consume ALL events to prevent them from affecting the next library
        int eventCount = 0;
        while (eventCount < 500) {  // Increased limit
            auto event = _window->pollEvent();
            if (!event) break;  // No more events
            eventCount++;
        }
        
        // Close the window and force immediate cleanup
        _window->close();
        
        // Extended delay to ensure SFML completely releases all resources
        sf::sleep(sf::milliseconds(200));
        
        // Final event cleanup after close
        eventCount = 0;
        while (eventCount < 100) {
            auto event = _window->pollEvent();
            if (!event) break;
            eventCount++;
        }
        
        // Delete the window object
        delete _window;
        _window = nullptr;
    }

    // Clear all SFML resources by creating a new default font
    // This ensures the old font resources are properly released
    _font = sf::Font();
    
    // Reset all other member variables to safe states
    _menuSystem = nullptr;
    _switchMessage.clear();
    _switchMessageTimer = 0;
    _targetFPS = 60;

    _initialized = false;
    
    // Clear any error state
    clearError();
}

void SFMLGraphics::render(const game_data& game) {
    if (!_initialized || !_window) {
        return;
    }

    // Clear screen with background color
    _window->clear(COLOR_BACKGROUND.toSFColor());

    // Check if we're in menu mode or game mode
    if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
        renderMenu();
    } else {
        // Render game
        int offsetX, offsetY, cellSize;
        calculateGameArea(game, offsetX, offsetY, cellSize);

        // Get game dimensions
        size_t width = game.get_width();
        size_t height = game.get_height();

        // Draw border
        drawRect(offsetX - 2, offsetY - 2, static_cast<int>(width) * cellSize + 4, static_cast<int>(height) * cellSize + 4, COLOR_BORDER, false);

        // Draw game board
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                int pixelX = offsetX + static_cast<int>(x) * cellSize;
                int pixelY = offsetY + static_cast<int>(y) * cellSize;

                // Check layer 2 first (food and snake)
                int layer2Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 2);
                
                if (layer2Value == FOOD) {
                    drawRect(pixelX, pixelY, cellSize, cellSize, COLOR_FOOD);
                } else if (layer2Value >= SNAKE_HEAD_PLAYER_1 && layer2Value < SNAKE_HEAD_PLAYER_1 + 1000000) {
                    // Snake head or body
                    if (layer2Value % 1000000 == 1) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, COLOR_SNAKE_HEAD);
                    } else {
                        drawRect(pixelX, pixelY, cellSize, cellSize, COLOR_SNAKE_BODY);
                    }
                } else {
                    // Check layer0 for walls and background
                    int layer0Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 0);
                    if (layer0Value == GAME_TILE_WALL) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, COLOR_BORDER);
                    } else if (layer0Value == GAME_TILE_ICE) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, COLOR_BACKGROUND);
                    }
                }
            }
        }

        // Draw score
        std::string scoreText = "Length: " + std::to_string(game.get_snake_length(0));
        drawText(scoreText, 10, 10, COLOR_TEXT, 20);
    }

    // Present the back buffer
    _window->display();
}

GameKey SFMLGraphics::getInput() {
    if (!_initialized || !_window) {
        return GameKey::NONE;
    }

    // SFML 3.x uses std::optional<Event> pollEvent()
    while (auto event = _window->pollEvent()) {
        // Check if it's a close event
        if (event->is<sf::Event::Closed>()) {
            _shouldContinue = false;
            return GameKey::QUIT;
        }
        
        // Check for key press events
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            GameKey key = translateSFMLKey(keyPressed->code);
            
            // Handle menu input if we're in menu mode (do NOT gate on key != NONE)
            if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
                // First, handle selection keys that aren't in our GameKey enum
                if (keyPressed->code == sf::Keyboard::Key::Enter ||
                    keyPressed->code == sf::Keyboard::Key::Space) {
                    _menuSystem->selectCurrentItem();
                    continue; // consume and keep polling
                }

                switch (key) {
                    case GameKey::UP:
                        _menuSystem->navigateUp();
                        break;
                    case GameKey::DOWN:
                        _menuSystem->navigateDown();
                        break;
                    case GameKey::ESCAPE:
                        _menuSystem->goBack();
                        break;
                    default:
                        // Return other keys (like 1/2/3) for library switching
                        return key;
                }
                // Continue polling events; do not return for handled menu input
                continue;
            } else {
                return key;
            }
        }
    }

    return GameKey::NONE;
}

const char* SFMLGraphics::getName() const {
    return "SFML Graphics Library";
}

bool SFMLGraphics::shouldContinue() const {
    return _shouldContinue && _initialized && _window && _window->isOpen();
}

const char* SFMLGraphics::getError() const {
    return _errorMessage.empty() ? nullptr : _errorMessage.c_str();
}

void SFMLGraphics::setFrameRate(int fps) {
    _targetFPS = fps;
    if (_window) {
        _window->setFramerateLimit(_targetFPS);
    }
}

void SFMLGraphics::setMenuSystem(MenuSystem* menuSystem) {
    _menuSystem = menuSystem;
}

void SFMLGraphics::setSwitchMessage(const std::string& message, int timer) {
    _switchMessage = message;
    _switchMessageTimer = timer;
}

// Private helper methods
void SFMLGraphics::setError(const std::string& error) {
    _errorMessage = error;
}

void SFMLGraphics::clearError() {
    _errorMessage.clear();
}

void SFMLGraphics::drawRect(int x, int y, int width, int height, const Color& color, bool filled) {
    if (!_window) return;

    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
    rect.setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));  // SFML 3.x uses Vector2f
    
    if (filled) {
        rect.setFillColor(color.toSFColor());
        rect.setOutlineThickness(0);
    } else {
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(color.toSFColor());
        rect.setOutlineThickness(2.0f);
    }
    
    _window->draw(rect);
}

void SFMLGraphics::drawText(const std::string& text, int x, int y, const Color& color, int fontSize) {
    if (!_window) return;

    // SFML 3.x requires font in Text constructor
    sf::Text sfText(_font, text, static_cast<unsigned int>(fontSize));
    sfText.setFillColor(color.toSFColor());
    sfText.setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y)));  // SFML 3.x uses Vector2f
    
    _window->draw(sfText);
}

GameKey SFMLGraphics::translateSFMLKey(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Up:      return GameKey::UP;
        case sf::Keyboard::Key::Down:    return GameKey::DOWN;
        case sf::Keyboard::Key::Left:    return GameKey::LEFT;
        case sf::Keyboard::Key::Right:   return GameKey::RIGHT;
        case sf::Keyboard::Key::Num1:    return GameKey::KEY_1;
        case sf::Keyboard::Key::Num2:    return GameKey::KEY_2;
        case sf::Keyboard::Key::Num3:    return GameKey::KEY_3;
        case sf::Keyboard::Key::Num4:    return GameKey::KEY_4;
        case sf::Keyboard::Key::Escape:  return GameKey::ESCAPE;
        default:                         return GameKey::NONE;
    }
}

void SFMLGraphics::calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize) {
    // Calculate the best cell size that fits the window
    size_t gameWidth = game.get_width();
    size_t gameHeight = game.get_height();
    
    int maxCellWidth = (WINDOW_WIDTH - 40) / static_cast<int>(gameWidth);   // Leave 20px margin on each side
    int maxCellHeight = (WINDOW_HEIGHT - 100) / static_cast<int>(gameHeight); // Leave space for UI

    cellSize = std::min(maxCellWidth, maxCellHeight);
    cellSize = std::max(cellSize, 8); // Minimum cell size

    // Center the game area
    int gameAreaWidth = static_cast<int>(gameWidth) * cellSize;
    int gameAreaHeight = static_cast<int>(gameHeight) * cellSize;
    
    offsetX = (WINDOW_WIDTH - gameAreaWidth) / 2;
    offsetY = 50 + (WINDOW_HEIGHT - 50 - gameAreaHeight) / 2; // 50px from top for UI
}

bool SFMLGraphics::initializeFont() {
    // Try to load a system font
    std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/Helvetica.ttc",           // macOS
        "/System/Library/Fonts/Supplemental/Arial.ttf",  // macOS alternative
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", // Linux
        "/usr/share/fonts/TTF/arial.ttf",                // Linux alternative
        "/Windows/Fonts/arial.ttf",                      // Windows
        "/System/Fonts/Helvetica.ttc",                   // macOS Big Sur+
        "C:\\Windows\\Fonts\\arial.ttf"                  // Windows alternative
    };

    for (const auto& fontPath : fontPaths) {
        if (_font.openFromFile(fontPath)) {  // SFML 3.x uses openFromFile instead of loadFromFile
            std::cout << "Loaded font: " << fontPath << std::endl;
            return true;
        }
    }

    setError("Failed to load any system font for SFML");
    return false;
}

void SFMLGraphics::drawCenteredText(const std::string& text, int y, const Color& color, int fontSize) {
    if (!_window) return;

    // SFML 3.x requires font in Text constructor
    sf::Text sfText(_font, text, static_cast<unsigned int>(fontSize));
    sfText.setFillColor(color.toSFColor());
    
    // Center horizontally - SFML 3.x getLocalBounds() returns different struct
    sf::FloatRect textBounds = sfText.getLocalBounds();
    float textWidth = textBounds.size.x;  // SFML 3.x uses .size.x instead of .width
    sfText.setPosition(sf::Vector2f((WINDOW_WIDTH - textWidth) / 2.0f, static_cast<float>(y)));
    
    _window->draw(sfText);
}

int SFMLGraphics::getTextWidth(const std::string& text, int fontSize) {
    // SFML 3.x requires font in Text constructor
    sf::Text sfText(_font, text, static_cast<unsigned int>(fontSize));
    
    // SFML 3.x uses .size.x instead of .width
    return static_cast<int>(sfText.getLocalBounds().size.x);
}

int SFMLGraphics::getTextHeight(int fontSize) {
    // SFML 3.x requires font in Text constructor
    sf::Text sfText(_font, "Tg", static_cast<unsigned int>(fontSize)); // Text with ascenders and descenders
    
    // SFML 3.x uses .size.y instead of .height
    return static_cast<int>(sfText.getLocalBounds().size.y);
}

// Menu rendering methods
void SFMLGraphics::renderMenu() {
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

void SFMLGraphics::renderMainMenu() {
    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, COLOR_SNAKE_HEAD, 48);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 200);

    // Draw instructions
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, COLOR_TEXT, 16);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, COLOR_TEXT, 16);
}

void SFMLGraphics::renderSettingsMenu() {
    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_SNAKE_HEAD, 36);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 120);
}

void SFMLGraphics::renderCreditsMenu() {
    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_SNAKE_HEAD, 36);

    // Draw credits content
    const auto& content = _menuSystem->getCreditsContent();
    int yPos = 120;
    for (const auto& line : content) {
        if (!line.empty()) {
            drawCenteredText(line, yPos, COLOR_TEXT, 16);
        }
        yPos += 25;
        if (yPos > WINDOW_HEIGHT - 100) break; // Don't overflow
    }

    // Draw back instruction
    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT - 60, COLOR_SNAKE_HEAD, 18);
}

void SFMLGraphics::renderInstructionsMenu() {
    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, COLOR_SNAKE_HEAD, 36);

    // Draw instructions content
    const auto& content = _menuSystem->getInstructionsContent();
    int yPos = 120;
    for (const auto& line : content) {
        if (!line.empty()) {
            drawCenteredText(line, yPos, COLOR_TEXT, 16);
        }
        yPos += 25;
        if (yPos > WINDOW_HEIGHT - 100) break; // Don't overflow
    }

    // Draw back instruction
    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT - 60, COLOR_SNAKE_HEAD, 18);
}

void SFMLGraphics::renderGameOverMenu() {
    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, COLOR_FOOD, 48);

    // Draw final score
    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredText(scoreText, 180, COLOR_TEXT, 24);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 240);
}

void SFMLGraphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY) {
    int yPos = startY;
    
    for (int i = 0; i < static_cast<int>(items.size()); i++) {
        const MenuItem& item = items[i];
        
        if (item.text.empty()) {
            yPos += 15; // Spacer
            continue;
        }
        
        Color textColor = COLOR_TEXT;
        int fontSize = 20;
        
        if (item.selectable && i == selectedIndex) {
            // Draw selection background
            int textWidth = getTextWidth(item.text, fontSize);
            int rectX = (WINDOW_WIDTH - textWidth - 20) / 2;
            drawRect(rectX, yPos - 5, textWidth + 20, fontSize + 10, COLOR_SELECTOR_BG);
            textColor = COLOR_SELECTED_TEXT;
        } else if (!item.selectable) {
            textColor = Color(150, 150, 150); // Gray for non-selectable items
            fontSize = 16;
        }
        
        drawCenteredText(item.text, yPos, textColor, fontSize);
        yPos += 35;
    }
}

// C interface for dynamic library loading
extern "C" {
    IGraphicsLibrary* createGraphicsLibrary() {
        return new SFMLGraphics();
    }

    void destroyGraphicsLibrary(IGraphicsLibrary* lib) {
        delete lib;
    }

    const char* getLibraryName() {
        return "SFML Graphics Library";
    }

    const char* getLibraryVersion() {
        return "1.0.0";
    }
}
