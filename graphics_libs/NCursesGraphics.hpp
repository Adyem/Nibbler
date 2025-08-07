#ifndef NCURSESGRAPHICS_HPP
#define NCURSESGRAPHICS_HPP

#include "../IGraphicsLibrary.hpp"
#include <ncurses.h>
#include <string>

class NCursesGraphics : public IGraphicsLibrary {
public:
    NCursesGraphics();
    virtual ~NCursesGraphics();

    // IGraphicsLibrary interface implementation
    virtual int initialize() override;
    virtual void shutdown() override;
    virtual void render(const game_data& game) override;
    virtual GameKey getInput() override;
    virtual const char* getName() const override;
    virtual bool shouldContinue() const override;
    virtual void setFrameRate(int fps) override;
    virtual const char* getError() const override;

private:
    bool _initialized;
    bool _shouldContinue;
    int _frameRate;
    std::string _errorMessage;
    WINDOW* _gameWindow;
    WINDOW* _infoWindow;

    // Color pairs
    enum ColorPairs {
        COLOR_SNAKE_HEAD = 1,
        COLOR_SNAKE_BODY = 2,
        COLOR_FOOD = 3,
        COLOR_WALL = 4,
        COLOR_ICE = 5,
        COLOR_BORDER = 6,
        COLOR_INFO = 7
    };

    // Helper methods
    void initializeColors();
    void drawBorder(const game_data& game);
    void drawGameArea(const game_data& game);
    void drawInfo(const game_data& game);
    char getCharFromGameTile(int x, int y, const game_data& game);
    int getColorFromGameTile(int x, int y, const game_data& game);
    void setError(const std::string& error);
    void clearError();
};

#endif // NCURSESGRAPHICS_HPP