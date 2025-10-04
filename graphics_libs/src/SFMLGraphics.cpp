#include "SFMLGraphics.hpp"
#include "../../game_data.hpp"
#include "../../MenuSystem.hpp"
#include <SFML/System/Sleep.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <thread>

// Static color definitions
const SFMLGraphics::Color SFMLGraphics::COLOR_BACKGROUND(20, 20, 30);  // Dark blue-gray
const SFMLGraphics::Color SFMLGraphics::COLOR_BORDER(100, 100, 120);   // Light gray
const SFMLGraphics::Color SFMLGraphics::COLOR_SNAKE_HEAD(50, 200, 50); // Bright green
const SFMLGraphics::Color SFMLGraphics::COLOR_SNAKE_BODY(30, 150, 30); // Dark green
const SFMLGraphics::Color SFMLGraphics::COLOR_FOOD(200, 50, 50);        // Red
const SFMLGraphics::Color SFMLGraphics::COLOR_TEXT(255, 255, 255);      // White
const SFMLGraphics::Color SFMLGraphics::COLOR_FIRE_FOOD(255, 140, 0);   // Bright orange
const SFMLGraphics::Color SFMLGraphics::COLOR_FROSTY_FOOD(80, 200, 235); // Cyan for frosty food
const SFMLGraphics::Color SFMLGraphics::COLOR_FIRE_TILE(200, 60, 40);   // Deep red for fire tiles

// Alternative palette
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_BACKGROUND(15, 15, 18);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_BORDER(180, 160, 90);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_SNAKE_HEAD(80, 180, 220);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_SNAKE_BODY(40, 120, 180);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_FOOD(235, 130, 35);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_TEXT(240, 240, 240);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_FIRE_FOOD(255, 165, 70);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_FROSTY_FOOD(110, 210, 245);
const SFMLGraphics::Color SFMLGraphics::ALT_COLOR_FIRE_TILE(220, 80, 55);

// Additional colors for better UI
const SFMLGraphics::Color SFMLGraphics::COLOR_SELECTOR_BG(70, 130, 180);    // Steel blue for selector
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

    // Create SFML window - SFML 2.5 uses width and height directly
    _window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Nibbler - SFML Graphics");
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

        // Force window to lose focus to help release graphics context
        _window->setActive(false);

        // Aggressively consume ALL events to prevent them from affecting the next library
        int eventCount = 0;
        sf::Event event;
        while (eventCount < 500) { // Increased limit
            if (!_window->pollEvent(event))
                break; // No more events
            eventCount++;
        }

        // Close the window and force immediate cleanup
        _window->close();

        // Extended delay to ensure SFML completely releases all resources
        sf::sleep(sf::milliseconds(200));

        // Final event cleanup after close
        eventCount = 0;
        while (eventCount < 100) {
            if (!_window->pollEvent(event))
                break;
            eventCount++;
        }

        // Delete the window object
        delete _window;
        _window = nullptr;

        // Additional delay specifically for OpenGL context cleanup
        sf::sleep(sf::milliseconds(200)); // Increased from 150ms

        // Force complete resource cleanup
        std::this_thread::yield();

        // Additional delay to ensure OpenGL driver has time to clean up
        sf::sleep(sf::milliseconds(150)); // Increased from 100ms

        // Extra aggressive cleanup for Raylib compatibility
        // Force OpenGL state reset with additional delays
        sf::sleep(sf::milliseconds(100));
        sf::sleep(sf::milliseconds(100));
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

    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& bg = useAlt ? ALT_COLOR_BACKGROUND : COLOR_BACKGROUND;
    const Color& border = useAlt ? ALT_COLOR_BORDER : COLOR_BORDER;
    const Color& head = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& body = useAlt ? ALT_COLOR_SNAKE_BODY : COLOR_SNAKE_BODY;
    const Color& food = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;
    const Color& fireFood = useAlt ? ALT_COLOR_FIRE_FOOD : COLOR_FIRE_FOOD;
    const Color& frostyFood = useAlt ? ALT_COLOR_FROSTY_FOOD : COLOR_FROSTY_FOOD;
    const Color& fireTile = useAlt ? ALT_COLOR_FIRE_TILE : COLOR_FIRE_TILE;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Clear screen with background color
    _window->clear(bg.toSFColor());

    bool menuActive = _menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME;

    // Check if we're in menu mode or game mode
    if (menuActive) {
        renderMenu();
    } else {
        // Render game
        int offsetX, offsetY, cellSize;
        calculateGameArea(game, offsetX, offsetY, cellSize);

        // Get game dimensions
        size_t width = game.get_width();
        size_t height = game.get_height();

        // Draw border (toggleable)
        bool showBorders = _menuSystem && _menuSystem->getSettings().showBorders;
        if (showBorders) {
            drawRect(offsetX - 2, offsetY - 2, static_cast<int>(width) * cellSize + 4, static_cast<int>(height) * cellSize + 4, border, false);
        }

        // Draw game board
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                int pixelX = offsetX + static_cast<int>(x) * cellSize;
                int pixelY = offsetY + static_cast<int>(y) * cellSize;

                // Check layer 2 first (food and snake)
                int layer2Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 2);

                if (layer2Value == FOOD) {
                    drawRect(pixelX, pixelY, cellSize, cellSize, food);
                } else if (layer2Value == FIRE_FOOD) {
                    drawRect(pixelX, pixelY, cellSize, cellSize, fireFood);
                } else if (layer2Value == FROSTY_FOOD) {
                    drawRect(pixelX, pixelY, cellSize, cellSize, frostyFood);
                } else if (layer2Value >= SNAKE_HEAD_PLAYER_1 && layer2Value < SNAKE_HEAD_PLAYER_1 + 1000000) {
                    // Snake head or body
                    if (layer2Value % 1000000 == 1) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, head);
                    } else {
                        drawRect(pixelX, pixelY, cellSize, cellSize, body);
                    }
                } else {
                    // Check layer0 for walls and background
                    int layer0Value = game.get_map_value(static_cast<int>(x), static_cast<int>(y), 0);
                    if (layer0Value == GAME_TILE_WALL) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, border);
                    } else if (layer0Value == GAME_TILE_FIRE) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, fireTile);
                    } else if (layer0Value == GAME_TILE_ICE) {
                        drawRect(pixelX, pixelY, cellSize, cellSize, bg);
                    }
                }
            }
        }

        // Draw score and optional FPS
        std::string scoreText = "Length: " + std::to_string(game.get_snake_length(0));
        drawText(scoreText, 10, 10, text, 20);
        if (_menuSystem && _menuSystem->getSettings().showFPS) {
            drawText(std::string("FPS: ") + std::to_string(_targetFPS), 10, 34, text, 16);
        }
    }

    if (_switchMessageTimer > 0) {
        if (!_switchMessage.empty()) {
            int bannerHeight = 110;
            int bannerY = WINDOW_HEIGHT / 2 - bannerHeight / 2;

            sf::RectangleShape banner(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(bannerHeight)));
            banner.setPosition(0.f, static_cast<float>(bannerY));
            banner.setFillColor(sf::Color(0, 0, 0, 200));
            _window->draw(banner);

            int fontSize = 30;
            int textWidth = getTextWidth(_switchMessage, fontSize);
            int textHeight = getTextHeight(fontSize);
            int textX = (WINDOW_WIDTH - textWidth) / 2;
            int textY = bannerY + (bannerHeight - textHeight) / 2;
            drawText(_switchMessage, textX, textY, text, fontSize);
        }

        _switchMessageTimer = std::max(0, _switchMessageTimer - 1);
    }

    // Present the back buffer
    _window->display();
}

GameKey SFMLGraphics::getInput() {
    if (!_initialized || !_window) {
        return GameKey::NONE;
    }

    // SFML 2.5 uses Event reference in pollEvent()
    sf::Event event;
    while (_window->pollEvent(event)) {
        // Check if it's a close event
        if (event.type == sf::Event::Closed) {
            _shouldContinue = false;
            return GameKey::QUIT;
        }

        // Check for key press events
        if (event.type == sf::Event::KeyPressed) {
            GameKey key = translateSFMLKey(event.key.code);

            // Handle menu input if we're in menu mode (do NOT gate on key != NONE)
            if (_menuSystem && _menuSystem->getCurrentState() != MenuState::IN_GAME) {
                // First, handle selection keys that aren't in our GameKey enum
                if (event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space) {
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
                    // Return other keys (like 1/2/3/4) for library switching
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
    if (!_window)
        return;

    sf::RectangleShape rect(sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
    rect.setPosition(sf::Vector2f(static_cast<float>(x), static_cast<float>(y))); // SFML 3.x uses Vector2f

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
    if (!_window)
        return;

    // SFML 2.5 requires text, font, and size separately
    sf::Text sfText;
    sfText.setString(text);
    sfText.setFont(_font);
    sfText.setCharacterSize(static_cast<unsigned int>(fontSize));
    sfText.setFillColor(color.toSFColor());
    sfText.setPosition(static_cast<float>(x), static_cast<float>(y));

    _window->draw(sfText);
}

GameKey SFMLGraphics::translateSFMLKey(sf::Keyboard::Key key) {
    switch (key) {
    case sf::Keyboard::Up:
        return GameKey::UP;
    case sf::Keyboard::Down:
        return GameKey::DOWN;
    case sf::Keyboard::Left:
        return GameKey::LEFT;
    case sf::Keyboard::Right:
        return GameKey::RIGHT;
    case sf::Keyboard::Num1:
        return GameKey::KEY_1;
    case sf::Keyboard::Num2:
        return GameKey::KEY_2;
    case sf::Keyboard::Num3:
        return GameKey::KEY_3;
    case sf::Keyboard::Num4:
        return GameKey::KEY_4;
    case sf::Keyboard::Escape:
        return GameKey::ESCAPE;
    default:
        return GameKey::NONE;
    }
}

void SFMLGraphics::calculateGameArea(const game_data& game, int& offsetX, int& offsetY, int& cellSize) {
    // Calculate the best cell size that fits the window
    size_t gameWidth = game.get_width();
    size_t gameHeight = game.get_height();

    int maxCellWidth = (WINDOW_WIDTH - 40) / static_cast<int>(gameWidth);     // Leave 20px margin on each side
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
        "/System/Library/Fonts/Helvetica.ttc",             // macOS
        "/System/Library/Fonts/Supplemental/Arial.ttf",    // macOS alternative
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", // Linux
        "/usr/share/fonts/TTF/arial.ttf",                  // Linux alternative
        "/Windows/Fonts/arial.ttf",                        // Windows
        "/System/Fonts/Helvetica.ttc",                     // macOS Big Sur+
        "C:\\Windows\\Fonts\\arial.ttf"                    // Windows alternative
    };

    for (const auto& fontPath : fontPaths) {
        if (_font.loadFromFile(fontPath)) { // SFML 2.5 uses loadFromFile
            std::cout << "Loaded font: " << fontPath << std::endl;
            return true;
        }
    }

    setError("Failed to load any system font for SFML");
    return false;
}

void SFMLGraphics::drawCenteredText(const std::string& text, int y, const Color& color, int fontSize) {
    if (!_window)
        return;

    // SFML 2.5 requires text, font, and size separately
    sf::Text sfText;
    sfText.setString(text);
    sfText.setFont(_font);
    sfText.setCharacterSize(static_cast<unsigned int>(fontSize));
    sfText.setFillColor(color.toSFColor());

    // Center horizontally - SFML 2.5 getLocalBounds() returns FloatRect with .width member
    sf::FloatRect textBounds = sfText.getLocalBounds();
    float textWidth = textBounds.width; // SFML 2.5 uses .width instead of .size.x
    sfText.setPosition((WINDOW_WIDTH - textWidth) / 2.0f, static_cast<float>(y));

    _window->draw(sfText);
}

int SFMLGraphics::getTextWidth(const std::string& text, int fontSize) {
    // SFML 2.5 requires text, font, and size separately
    sf::Text sfText;
    sfText.setString(text);
    sfText.setFont(_font);
    sfText.setCharacterSize(static_cast<unsigned int>(fontSize));

    // SFML 2.5 uses .width
    return static_cast<int>(sfText.getLocalBounds().width);
}

int SFMLGraphics::getTextHeight(int fontSize) {
    // SFML 2.5 requires text, font, and size separately
    sf::Text sfText;
    sfText.setString("Tg"); // Text with ascenders and descenders
    sfText.setFont(_font);
    sfText.setCharacterSize(static_cast<unsigned int>(fontSize));

    // SFML 2.5 uses .height
    return static_cast<int>(sfText.getLocalBounds().height);
}

// Menu rendering methods
void SFMLGraphics::renderMenu() {
    if (!_menuSystem)
        return;

    bool useAlt = _menuSystem->getSettings().useAlternativeColors;
    const Color& bg = useAlt ? ALT_COLOR_BACKGROUND : COLOR_BACKGROUND;

    // Clear with palette background
    _window->clear(bg.toSFColor());

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
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, title, 48);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 200);

    // Draw instructions
    drawCenteredText("Use Arrow Keys to navigate, ENTER to select", WINDOW_HEIGHT - 80, text, 16);
    drawCenteredText("Press 1/2/3/4 to switch graphics libraries", WINDOW_HEIGHT - 60, text, 16);
}

void SFMLGraphics::renderSettingsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 120);

    // Footer colors
    drawCenteredText("Use Arrow Keys to navigate, ENTER to toggle/adjust", WINDOW_HEIGHT - 80, text, 16);
    drawCenteredText("ESC to go back", WINDOW_HEIGHT - 60, text, 16);
}

void SFMLGraphics::renderCreditsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);

    // Two-column content with forced split at "BONUS FEATURES:"
    const auto& content = _menuSystem->getCreditsContent();
    int top = 120;
    int lineH = std::max(22, getTextHeight(18) + 6);
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
            drawText(content[i], colX1, y1, text, 18);
            y1 += lineH;
        }
        int y2 = top;
        for (size_t i = bonusIdx; i < content.size() && y2 <= bottomY - lineH; ++i) {
            if (content[i].empty()) {
                y2 += lineH;
                continue;
            }
            drawText(content[i], colX2, y2, text, 18);
            y2 += lineH;
        }
    } else {
        // Fallback auto two-column flow
        int usableH = bottomY - top - 20;
        int linesPerCol = std::max(1, usableH / lineH);
        for (size_t i = 0; i < content.size(); ++i) {
            int col = static_cast<int>(i) / linesPerCol;
            int row = static_cast<int>(i) % linesPerCol;
            int x = (col % 2 == 0) ? colX1 : colX2;
            int y = top + row * lineH;
            if (y > bottomY)
                continue;
            drawText(content[i], x, y, text, 18);
        }
    }

    // Footer
    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT - 60, title, 18);
}

void SFMLGraphics::renderInstructionsMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_SNAKE_HEAD : COLOR_SNAKE_HEAD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 60, title, 36);

    // Two-column content
    const auto& content = _menuSystem->getInstructionsContent();
    int top = 120;
    int lineH = std::max(22, getTextHeight(18) + 6);
    int usableH = WINDOW_HEIGHT - top - 100;
    int linesPerCol = std::max(1, usableH / lineH);

    int colX1 = 80;
    int colX2 = WINDOW_WIDTH / 2 + 40;

    for (size_t i = 0; i < content.size(); ++i) {
        int col = static_cast<int>(i) / linesPerCol;
        int row = static_cast<int>(i) % linesPerCol;
        int x = (col % 2 == 0) ? colX1 : colX2;
        int y = top + row * lineH;
        if (y > WINDOW_HEIGHT - 100)
            continue;
        drawText(content[i], x, y, text, 18);
    }

    // Footer
    drawCenteredText("Press ESC or ENTER to go back", WINDOW_HEIGHT - 60, title, 18);
}

void SFMLGraphics::renderGameOverMenu() {
    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& title = useAlt ? ALT_COLOR_FOOD : COLOR_FOOD;
    const Color& text = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    // Draw title
    drawCenteredText(_menuSystem->getCurrentTitle(), 100, title, 48);

    // Draw game over message
    drawCenteredText("Your snake has collided!", 140, title, 24);

    // Draw final score
    std::string scoreText = "Final Score: " + std::to_string(_menuSystem->getGameOverScore());
    drawCenteredText(scoreText, 180, text, 24);

    // Draw menu items
    const auto& items = _menuSystem->getCurrentMenuItems();
    drawMenuItems(items, _menuSystem->getCurrentSelection(), 240);
}

void SFMLGraphics::drawMenuItems(const std::vector<MenuItem>& items, int selectedIndex, int startY) {
    int yPos = startY;

    bool useAlt = _menuSystem && _menuSystem->getSettings().useAlternativeColors;
    const Color& textColorBase = useAlt ? ALT_COLOR_TEXT : COLOR_TEXT;

    for (int i = 0; i < static_cast<int>(items.size()); i++) {
        const MenuItem& item = items[i];

        if (item.text.empty()) {
            yPos += 15; // Spacer
            continue;
        }

        Color textColor = textColorBase;
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
