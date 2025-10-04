#define private public
#include "../GameEngine.hpp"
#undef private

#include <cassert>
#include <iostream>

static void verify_tile(const game_data &data, int x, int y, int expected, const char *label) {
    int value = data.get_map_value(x, y, 0);
    if (value != expected) {
        std::cerr << label << " expected tile " << expected << " but found " << value << std::endl;
    }
    assert(value == expected);
}

static void test_bonus_map_persists_across_game_over() {
    GameEngine engine(10, 10);
    int loadResult = engine.loadBonusMap("Test/maps/persistence_bonus.nib");
    assert(loadResult == 0);
    assert(engine._usingBonusMap);
    assert(engine._cachedBonusRules.has_value());

    // Coordinates for the custom fire tile and an interior wall
    const int fireX = 7;
    const int fireY = 1;
    const int wallX = 0;
    const int wallY = 0;

    verify_tile(engine._gameData, fireX, fireY, GAME_TILE_FIRE, "initial fire tile");
    verify_tile(engine._gameData, wallX, wallY, GAME_TILE_WALL, "initial boundary wall");

    // Simulate gameplay modifying the board
    engine._gameData.set_map_value(fireX, fireY, 0, GAME_TILE_EMPTY);
    verify_tile(engine._gameData, fireX, fireY, GAME_TILE_EMPTY, "fire tile cleared during play");

    // Trigger game over flow directly
    engine.handleGameOver();

    // The bonus rules should have been re-applied
    verify_tile(engine._gameData, fireX, fireY, GAME_TILE_FIRE, "fire tile restored after game over");
    verify_tile(engine._gameData, wallX, wallY, GAME_TILE_WALL, "wall preserved after game over");

    const GameSettings &settings = engine._menuSystem.getSettings();
    assert(settings.wrapAroundEdges == false);
    assert(settings.additionalFoodItems == true);
    assert(engine._menuSystem.isBonusFeaturesAvailable());
    assert(engine._usingBonusMap);

    std::cout << "Bonus map persistence regression test passed" << std::endl;
}

static void test_snake_length_resets_after_game_over() {
    GameEngine engine(10, 10);
    int loadResult = engine.loadBonusMap("Test/maps/persistence_bonus.nib");
    assert(loadResult == 0);
    int startingLength = engine._gameData.get_snake_length(0);
    assert(startingLength > 0);

    engine._gameData.set_player_snake_length(0, startingLength + 5);
    engine._gameData.set_player_snake_length(1, 3);
    engine._gameData.set_player_snake_length(2, 2);
    engine._gameData.set_player_snake_length(3, 1);

    assert(engine._gameData.get_snake_length(0) == startingLength + 5);
    assert(engine._gameData.get_snake_length(1) == 3);
    assert(engine._gameData.get_snake_length(2) == 2);
    assert(engine._gameData.get_snake_length(3) == 1);

    engine.handleGameOver();

    assert(engine._gameData.get_snake_length(0) == startingLength);
    assert(engine._gameData.get_snake_length(1) == 0);
    assert(engine._gameData.get_snake_length(2) == 0);
    assert(engine._gameData.get_snake_length(3) == 0);

    std::cout << "Snake length reset regression test passed" << std::endl;
}

int main() {
    test_bonus_map_persists_across_game_over();
    test_snake_length_resets_after_game_over();
    return 0;
}
