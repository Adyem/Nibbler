#include "../game_data.hpp"

#include <cassert>
#include <chrono>
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
    data.sync_snake_segments_from_map();
}

static void place_head(game_data &data, int head_value, int x, int y)
{
    data.set_map_value(x, y, 0, GAME_TILE_EMPTY);
    data.set_map_value(x, y, 2, head_value);
    data.sync_snake_segments_from_map();
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

static void test_high_length_snake_moves()
{
    const int width = 1500;
    const int height = 5;
    const int snakeLength = 1200;
    const int movesToRun = 200;
    const int row = height / 2;

    game_data data(width, height);
    clear_board(data);

    int head_x = snakeLength + 5;
    for (int i = 0; i < snakeLength; ++i)
    {
        int x = head_x - i;
        data.set_map_value(x, row, 0, GAME_TILE_EMPTY);
        data.set_map_value(x, row, 2, SNAKE_HEAD_PLAYER_1 + i);
    }
    data.set_player_snake_length(0, snakeLength);
    data.sync_snake_segments_from_map();

    data.set_direction_moving(0, DIRECTION_RIGHT);

    double step = 1.0 / data.get_moves_per_second();
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < movesToRun; ++i)
    {
        assert(data.test_is_valid_move(SNAKE_HEAD_PLAYER_1) == 0);
        int result = data.update_game_map(step);
        assert(result == 0);
    }
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    assert(duration.count() < 1500);

    assert(data.get_snake_length(0) == snakeLength);
    t_coordinates head = data.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    assert(head.x == head_x + movesToRun);
    assert(head.y == row);

    int firstTailX = head_x - snakeLength + 1;
    for (int moveIndex = 0; moveIndex < movesToRun; ++moveIndex)
    {
        int x = firstTailX + moveIndex;
        assert(data.get_map_value(x, row, 2) == 0);
    }

    std::cout << "High length movement test duration: " << duration.count() << "ms\n";
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

    test_high_length_snake_moves();

    std::cout << "Movement tests passed" << std::endl;
    return 0;
}
