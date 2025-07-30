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
