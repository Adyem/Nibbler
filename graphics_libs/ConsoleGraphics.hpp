#ifndef CONSOLEGRAPHICS_HPP
#define CONSOLEGRAPHICS_HPP

#include "../IGraphicsLibrary.hpp"
#include <termios.h>
#include <string>

class ConsoleGraphics : public IGraphicsLibrary {
public:
    ConsoleGraphics();
    virtual ~ConsoleGraphics();

    // IGraphicsLibrary interface implementation
    virtual int initialize() override;
    virtual void shutdown() override;
    virtual void render(const game_data& game) override;
    virtual GameKey getInput() override;
    virtual const char* getName() const override;
    virtual bool shouldContinue() const override;
    virtual const char* getError() const override;

private:
    bool _initialized;
    bool _shouldContinue;
    std::string _errorMessage;
    struct termios _originalTermios;

    // Helper methods
    void clearScreen();
    void setCursorPosition(int x, int y);
    void setNonBlockingInput();
    void restoreTerminalSettings();
    char getCharFromGameTile(int x, int y, const game_data& game);
    void setError(const std::string& error);
};

#endif // CONSOLEGRAPHICS_HPP