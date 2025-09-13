#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#define GAME_DATA_HPP
enum : char {
    MAP_TILE_EMPTY       = '0',
    MAP_TILE_WALL        = '1',
    MAP_TILE_ICE         = '2',
    MAP_TILE_FIRE        = '3',
    MAP_TILE_SNAKE_HEAD  = '4',
    MAP_TILE_SNAKE_BODY_1 = '5',
    MAP_TILE_SNAKE_BODY_2 = '6',
    MAP_TILE_SNAKE_BODY_3 = '7',
};

#include "../map_validation.cpp"

static void run_test(const std::string& name, bool result) {
    std::cout << name << ": " << (result ? "PASS" : "FAIL") << std::endl;
    assert(result);
}

int main() {
    // Valid map where all reachable tiles are connected
    std::vector<std::string> valid_map = {
        "111",
        "401",
        "111"
    };
    run_test("valid map connectivity", validate_map_path(valid_map, false));

    // Invalid map with an unreachable open tile
    std::vector<std::string> invalid_map = {
        "111",
        "410",
        "111"
    };
    run_test("unreachable tile map", !validate_map_path(invalid_map, false));

    // Map requiring edge wrapping to reach all tiles
    std::vector<std::string> wrap_map = {
        "410"
    };
    run_test("wrap map with wrapping", validate_map_path(wrap_map, true));
    run_test("wrap map without wrapping", !validate_map_path(wrap_map, false));

    // Map with no snake head should be invalid
    std::vector<std::string> no_head_map = {
        "111",
        "010",
        "111"
    };
    run_test("map without head", !validate_map_path(no_head_map, false));

    // Empty map should be invalid
    std::vector<std::string> empty_map;
    run_test("empty map", !validate_map_path(empty_map, false));

    // Map with valid head-to-tail path covering all non-wall tiles
    std::vector<std::string> head_tail_map = {
        "1111",
        "4001",
        "1111"
    };
    run_test("valid head-to-tail path", validate_head_to_tail_path(head_tail_map, false, 0, 1, 2, 1));

    // Map without a valid head-to-tail path
    std::vector<std::string> no_tail_path = {
        "1111",
        "4101",
        "1111"
    };
    run_test("missing head-to-tail path", !validate_head_to_tail_path(no_tail_path, false, 0, 1, 2, 1));

    // Head-to-tail path requiring edge wrapping
    run_test("head-to-tail with wrapping", validate_head_to_tail_path(wrap_map, true, 0, 0, 2, 0));
    run_test("head-to-tail without wrapping", !validate_head_to_tail_path(wrap_map, false, 0, 0, 2, 0));

    // Head-to-tail path over ice tiles
    std::vector<std::string> ice_path_map = {
        "42200"
    };
    run_test("head-to-tail across ice", validate_head_to_tail_path(ice_path_map, false, 0, 0, 4, 0));

    // Invalid head-to-tail path due to wall after ice
    std::vector<std::string> blocked_ice_map = {
        "42210"
    };
    run_test("blocked ice path", !validate_head_to_tail_path(blocked_ice_map, false, 0, 0, 4, 0));

    // Head-to-tail path impossible on a small open board
    std::vector<std::string> small_map = {
        "40",
        "00"
    };
    run_test("small board no path", !validate_head_to_tail_path(small_map, false, 0, 0, 1, 1));

    // Valid snake chain adjacency
    run_test("valid snake chain", validate_snake_chain(1, 1, 1, 2, 1, 3, 1, 4, 10, 10, false));

    // Invalid snake chain adjacency
    run_test("diagonal snake chain", !validate_snake_chain(1, 1, 2, 2, 3, 3, 4, 4, 10, 10, false));

    // Invalid snake chain due to overlapping segments
    run_test("overlapping snake segments", !validate_snake_chain(2, 2, 2, 2, 3, 3, 4, 4, 10, 10, false));

    // Snake chain adjacency across wrapped edges
    run_test("snake chain across wrap", validate_snake_chain(0, 0, 4, 0, 4, 4, 0, 4, 5, 5, true));
    run_test("snake chain across wrap disabled", !validate_snake_chain(0, 0, 4, 0, 4, 4, 0, 4, 5, 5, false));

    // Bigger map with multiple obstacles
    std::vector<std::string> big_map = {
        "1111111111",
        "1000000001",
        "1040000001",
        "1050000001",
        "1060000001",
        "1070000001",
        "1002003001",
        "1000000001",
        "1000000001",
        "1111111111"
    };
    run_test("big map connectivity", validate_map_path(big_map, false));
    run_test("big map head to tail", validate_head_to_tail_path(big_map, false, 2, 2, 2, 5));

    // Semi-complex map with maze-like corridors
    std::vector<std::string> maze_map = {
        "1111111",
        "1400001",
        "1011101",
        "1000101",
        "1010111",
        "1000001",
        "1111111"
    };
    run_test("maze map connectivity", validate_map_path(maze_map, false));

    // Semi-complex map containing unreachable pocket
    std::vector<std::string> unreachable_map = {
        "11111",
        "14011",
        "10101",
        "10101",
        "11111"
    };
    run_test("unreachable pocket map", !validate_map_path(unreachable_map, false));

    std::cout << "All tests passed" << std::endl;
    return 0;
}
