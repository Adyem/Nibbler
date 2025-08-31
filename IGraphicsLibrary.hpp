#pragma once

#include <string>

class game_data;
class MenuSystem;

enum class GameKey {
    NONE = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    ESCAPE,
    QUIT
};

class IGraphicsLibrary {
  public:
    virtual ~IGraphicsLibrary() = default;
    virtual int initialize() = 0;
    virtual void shutdown() = 0;
    virtual void render(const game_data& game) = 0;
    virtual GameKey getInput() = 0;
    virtual const char* getName() const = 0;
    virtual bool shouldContinue() const = 0;
    virtual void setFrameRate(int fps) {
        (void)fps;
    }
    virtual const char* getError() const {
        return NULL;
    }
    virtual void setMenuSystem(MenuSystem* menuSystem) {
        (void)menuSystem;
    }
    virtual void setSwitchMessage(const std::string& message, int timer) {
        (void)message;
        (void)timer;
    }
};

extern "C" {
IGraphicsLibrary* createGraphicsLibrary();
void destroyGraphicsLibrary(IGraphicsLibrary* lib);
const char* getLibraryName();
const char* getLibraryVersion();
}