#include "tests.hpp"
#include "game_data.hpp"
#include <iostream>
#include <iomanip>

static void print_layer(const game_data &gd) {
    for (size_t y = 0; y < gd._map.get_height(); ++y) {
        for (size_t x = 0; x < gd._map.get_width(); ++x) {
            std::cout << std::setw(8) << gd._map.get(x, y, 2);
        }
        std::cout << "\n";
    }
}

int test_game_data() {
    game_data gd(3, 3);
    if (gd._error) {
        std::cerr << "Initialization error: " << gd._error << std::endl;
        return 1;
    }
    gd._map.set(0, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd._direction_moving[0] = DIRECTION_RIGHT;
    if (gd.update_snake_position(SNAKE_HEAD_PLAYER_1)) {
        std::cerr << "Update failed" << std::endl;
        return 1;
    }
    print_layer(gd);
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 1 || head.y != 0) {
        std::cerr << "Unexpected head position: (" << head.x << "," << head.y << ")" << std::endl;
        return 1;
    }
    return 0;
}

int test_wrap_around_edges() {
    game_data gd(2, 2);
    if (gd._error)
        return 1;
    gd._wrap_around_edges = 1;
    gd._map.set(1, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd._direction_moving[0] = DIRECTION_RIGHT;
    if (gd.update_snake_position(SNAKE_HEAD_PLAYER_1))
        return 1;
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 0 || head.y != 0)
        return 1;
    return 0;
}

int test_invalid_move_wall() {
    game_data gd(2, 2);
    if (gd._error)
        return 1;
    gd._map.set(1, 0, 0, GAME_TILE_WALL);
    gd._map.set(0, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd._direction_moving[0] = DIRECTION_RIGHT;
    if (gd.is_valid_move(SNAKE_HEAD_PLAYER_1) == 0)
        return 1;
    if (gd.update_snake_position(SNAKE_HEAD_PLAYER_1) == 0)
        return 1;
    return 0;
}

int test_self_collision() {
    game_data gd(3, 1);
    if (gd._error)
        return 1;
    gd._map.set(1, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd._map.set(0, 0, 2, SNAKE_HEAD_PLAYER_1 + 1);
    gd._direction_moving[0] = DIRECTION_LEFT;
    if (gd.is_valid_move(SNAKE_HEAD_PLAYER_1) == 0)
        return 1;
    if (gd.update_snake_position(SNAKE_HEAD_PLAYER_1) == 0)
        return 1;
    return 0;
}

int run_all_tests() {
    int failed = 0;
    failed += test_game_data();
    failed += test_wrap_around_edges();
    failed += test_invalid_move_wall();
    failed += test_self_collision();
    return failed;
}

int run_all_tests_with_report() {
    int failed = run_all_tests();
    if (failed != 0) {
        std::cerr << "Tests failed" << std::endl;
    } else {
        std::cout << "All tests passed" << std::endl;
    }
    return failed;
}
