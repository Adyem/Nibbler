#include "../game_data.hpp"

#include <cassert>
#include <iostream>

static void clear_board(game_data &data)
{
    for (size_t y = 0; y < data.get_height(); ++y)
    {
        for (size_t x = 0; x < data.get_width(); ++x)
        {
            data.set_map_value(static_cast<int>(x), static_cast<int>(y), 0, GAME_TILE_EMPTY);
            data.set_map_value(static_cast<int>(x), static_cast<int>(y), 1, 0);
            data.set_map_value(static_cast<int>(x), static_cast<int>(y), 2, 0);
        }
    }
}

static void place_head(game_data &data, int head_value, int x, int y)
{
    data.set_map_value(x, y, 0, GAME_TILE_EMPTY);
    data.set_map_value(x, y, 2, head_value);
}

static void place_wall(game_data &data, int x, int y)
{
    data.set_map_value(x, y, 2, 0);
    data.set_map_value(x, y, 0, GAME_TILE_WALL);
}

static void ensure_empty(game_data &data, int x, int y)
{
    data.set_map_value(x, y, 0, GAME_TILE_EMPTY);
    data.set_map_value(x, y, 2, 0);
}

static void expect_valid_move(game_data &data, int head_value, const char *label)
{
    int result = data.test_is_valid_move(head_value);
    if (result != 0)
        std::cerr << label << " should have been valid but returned " << result << '\n';
    assert(result == 0);
}

int main()
{
    game_data data(5, 5);
    clear_board(data);

    // Set player 1 to move upward so aliasing would fail for other players
    data.set_direction_moving(0, DIRECTION_UP);

    // Player 2: move right into an empty tile, but up is blocked by a wall
    place_head(data, SNAKE_HEAD_PLAYER_2, 1, 1);
    place_wall(data, 1, 0);
    ensure_empty(data, 2, 1);
    data.set_direction_moving(1, DIRECTION_RIGHT);
    expect_valid_move(data, SNAKE_HEAD_PLAYER_2, "player 2 move");

    // Player 3: move down into an empty tile, but up is blocked by a wall
    place_head(data, SNAKE_HEAD_PLAYER_3, 3, 1);
    place_wall(data, 3, 0);
    ensure_empty(data, 3, 2);
    data.set_direction_moving(2, DIRECTION_DOWN);
    expect_valid_move(data, SNAKE_HEAD_PLAYER_3, "player 3 move");

    // Player 4: move left into an empty tile, but up is blocked by a wall
    place_head(data, SNAKE_HEAD_PLAYER_4, 2, 3);
    place_wall(data, 2, 2);
    ensure_empty(data, 1, 3);
    data.set_direction_moving(3, DIRECTION_LEFT);
    expect_valid_move(data, SNAKE_HEAD_PLAYER_4, "player 4 move");

    std::cout << "Movement tests passed" << std::endl;
    return 0;
}
