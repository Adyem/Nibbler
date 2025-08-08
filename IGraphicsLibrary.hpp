#ifndef IGRAPHICSLIBRARY_HPP
#define IGRAPHICSLIBRARY_HPP

#include <string>

// Forward declarations
class game_data;
class MenuSystem;

// Input keys that the game recognizes
enum class GameKey {
    NONE = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    KEY_1,      // Switch to library 1
    KEY_2,      // Switch to library 2
    KEY_3,      // Switch to library 3
    ESCAPE,     // Quit game
    QUIT
};

// Abstract interface that all graphics libraries must implement
class IGraphicsLibrary {
public:
    virtual ~IGraphicsLibrary() = default;

    // Initialize the graphics library
    // Returns 0 on success, non-zero on error
    virtual int initialize() = 0;

    // Clean up and shutdown the graphics library
    virtual void shutdown() = 0;

    // Display the current game state
    // game_data contains the complete game state including snake positions, food, etc.
    virtual void render(const game_data& game) = 0;

    // Get the next input from the user
    // Returns GameKey enum value
    // Should be non-blocking - return GameKey::NONE if no input available
    virtual GameKey getInput() = 0;

    // Get the name/identifier of this graphics library
    virtual const char* getName() const = 0;

    // Check if the library should continue running
    // Returns false if user wants to quit
    virtual bool shouldContinue() const = 0;

    // Set the target frame rate (optional, for libraries that support it)
    virtual void setFrameRate(int fps) { (void)fps; }

    // Get any error message from the library
    virtual const char* getError() const { return nullptr; }

    // Set menu system (optional, for libraries that support menus)
    virtual void setMenuSystem(MenuSystem* menuSystem) { (void)menuSystem; }

    // Set switch message to display (optional, for libraries that support it)
    virtual void setSwitchMessage(const std::string& message, int timer) { (void)message; (void)timer; }
};

// C interface for dynamic library loading
extern "C" {
    // Factory function that each dynamic library must implement
    // Returns a new instance of the graphics library
    IGraphicsLibrary* createGraphicsLibrary();

    // Cleanup function that each dynamic library must implement
    // Destroys the graphics library instance
    void destroyGraphicsLibrary(IGraphicsLibrary* lib);

    // Get library information
    const char* getLibraryName();
    const char* getLibraryVersion();
}

#endif // IGRAPHICSLIBRARY_HPP