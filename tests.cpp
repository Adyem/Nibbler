#include "tests.hpp"
#include "game_data.hpp"
#include <iostream>
#include <iomanip>

static void print_layer(const game_data &gd)
{
    size_t y = 0;
    while (y < gd.get_height())
    {
        size_t x = 0;
        while (x < gd.get_width())
        {
            std::cout << std::setw(8) << gd.get_map_value(static_cast<int>(x),
					static_cast<int>(y), 2);
            ++x;
        }
        std::cout << "\n";
        ++y;
    }
	return ;
}

static void perform_updates(game_data &gd)
{
    int i = 0;
    while (i < 60)
    {
        gd.update_game_map();
        ++i;
    }
    return ;
}

static int test_game_data()
{
    game_data gd(3, 3);
    if (gd.get_error())
    {
        std::cerr << "Initialization error: " << gd.get_error() << std::endl;
        return (1);
    }
    gd.set_map_value(0, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd.set_direction_moving(0, DIRECTION_RIGHT);
    perform_updates(gd);
    print_layer(gd);
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 1 || head.y != 0)
	{
        std::cerr << "Unexpected head position: (" << head.x << "," << head.y << ")"
			<< std::endl;
        return (1);
    }
    return (0);
}

static int test_wrap_around_edges()
{
    game_data gd(2, 2);
    if (gd.get_error())
        return (1);
    gd.set_wrap_around_edges(1);
    gd.set_map_value(1, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd.set_direction_moving(0, DIRECTION_RIGHT);
    perform_updates(gd);
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 0 || head.y != 0)
        return (1);
    return (0);
}

static int test_invalid_move_wall()
{
    game_data gd(2, 2);
    if (gd.get_error())
        return (1);
    gd.set_map_value(1, 0, 0, GAME_TILE_WALL);
    gd.set_map_value(0, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd.set_direction_moving(0, DIRECTION_RIGHT);
    if (gd.test_is_valid_move(SNAKE_HEAD_PLAYER_1) == 0)
        return (1);
    perform_updates(gd);
    return (0);
}

static int test_self_collision()
{
    game_data gd(3, 1);
    if (gd.get_error())
        return (1);
    gd.set_map_value(1, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd.set_map_value(0, 0, 2, SNAKE_HEAD_PLAYER_1 + 1);
    gd.set_direction_moving(0, DIRECTION_LEFT);
    if (gd.test_is_valid_move(SNAKE_HEAD_PLAYER_1) == 0)
        return (1);
    perform_updates(gd);
    return (0);
}

static int test_reset_board() {
    game_data gd(5, 5);
    if (gd.get_error())
        return (1);
    gd.reset_board();
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 2 || head.y != 2)
        return (1);
    return (0);
}

static int test_resize_board() {
    game_data gd(3, 3);
    if (gd.get_error())
        return (1);
    gd.resize_board(4, 4);
    if (gd.get_width() != 4 || gd.get_height() != 4)
        return (1);
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 2 || head.y != 2)
        return (1);
    return (0);
}

static int test_eat_food() {
    game_data gd(3, 1);
    if (gd.get_error())
        return (1);
    gd.set_map_value(0, 0, 2, SNAKE_HEAD_PLAYER_1);
    gd.set_map_value(1, 0, 2, FOOD);
    gd.set_direction_moving(0, DIRECTION_RIGHT);
    perform_updates(gd);
    if (gd.get_map_value(0, 0, 2) == 0)
        return (1);
    t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
    if (head.x != 1 || head.y != 0)
        return (1);
    bool food_found = false;
    for (size_t y = 0; y < gd.get_height(); ++y)
        for (size_t x = 0; x < gd.get_width(); ++x)
            if (gd.get_map_value(static_cast<int>(x), static_cast<int>(y), 2) == FOOD)
                food_found = true;
    if (!food_found)
        return (1);
    return (0);
}

static int test_save_load_and_achievement()
{
    game_data gd(100, 1);
    if (gd.get_error())
        return (1);
    ft_string profile("profile1");
    gd.set_profile_name(profile);
    for (int i = 0; i < 49; ++i)
    {
        t_coordinates head = gd.get_head_coordinate(SNAKE_HEAD_PLAYER_1);
        gd.set_map_value(head.x + 1, head.y, 2, FOOD);
        gd.set_direction_moving(0, DIRECTION_RIGHT);
        perform_updates(gd);
        for (size_t x = 0; x < gd.get_width(); ++x)
            if (gd.get_map_value(static_cast<int>(x), 0, 2) == FOOD)
                gd.set_map_value(static_cast<int>(x), 0, 2, 0);
    }
    if (gd.get_snake_length(0) != 50)
        return (1);
    if (!gd.get_achievement_snake50())
        return (1);
    if (gd.save_game())
        return (1);
    game_data gd2(100, 1);
    if (gd2.get_error())
        return (1);
    gd2.set_profile_name(profile);
    if (gd2.load_game())
        return (1);
    if (gd2.get_snake_length(0) != 50)
        return (1);
    if (!gd2.get_achievement_snake50())
        return (1);
    return (0);
}

int run_all_tests_with_report()
{
    struct test_entry
	{
        const char *name;
        int (*func)();
    }
	tests[] =
	{
        {"test_game_data", test_game_data},
        {"test_wrap_around_edges", test_wrap_around_edges},
        {"test_invalid_move_wall", test_invalid_move_wall},
        {"test_self_collision", test_self_collision},
        {"test_reset_board", test_reset_board},
        {"test_resize_board", test_resize_board},
        {"test_eat_food", test_eat_food},
        {"test_save_load_and_achievement", test_save_load_and_achievement},
    };

    int failed = 0;
    size_t idx = 0;
    while (idx < sizeof(tests) / sizeof(tests[0]))
    {
        const auto &t = tests[idx];
        int ret = t.func();
        if (ret)
            std::cerr << t.name << " failed" << std::endl;
        else
            std::cout << t.name << " passed" << std::endl;
        failed += ret;
        ++idx;
    }
	if (failed != 0)
        std::cerr << "Tests failed: " << failed << " test(s)" << std::endl;
	else
        std::cout << "All tests passed" << std::endl;
	return (failed);
}
