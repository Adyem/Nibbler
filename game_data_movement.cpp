#include "game_data.hpp"
#include <climits>

t_coordinates game_data::get_head_coordinate(int head_to_find) {
    size_t index_y = 0;
    while (index_y < this->_map.get_height()) {
        size_t index_x = 0;
        while (index_x < this->_map.get_width()) {
            if (this->_map.get(index_x, index_y, 2) == head_to_find)
                return ((t_coordinates){static_cast<int>(index_x),
                                        static_cast<int>(index_y)});
            index_x++;
        }
        index_y++;
    }
    return ((t_coordinates){0, 0});
}

int game_data::determine_player_number(int player_head) {
    return ((player_head % 1000000) - 1);
}

int game_data::is_valid_move(int player_head) {
    t_coordinates head = this->get_head_coordinate(player_head);
    int player_number = this->determine_player_number(player_head);

    int direction_moving = this->_direction_moving[player_number];
    if (this->_frosty_steps[player_number] > 0 ||
        this->_map.get(head.x, head.y, 0) == GAME_TILE_ICE)
        direction_moving = this->_direction_moving_ice[player_number];

    // If no direction is set, the move is valid but the snake won't move
    // This is handled in update_snake_position()
    if (direction_moving == DIRECTION_NONE)
        return (0);

    int width = static_cast<int>(this->_map.get_width());
    int height = static_cast<int>(this->_map.get_height());

    if (head.x < 0 || head.x >= width || head.y < 0 || head.y >= height)
        return (1);

    int target_x = head.x;
    int target_y = head.y;

    if (direction_moving == DIRECTION_UP) {
        target_y = head.y - 1; // UP decreases Y coordinate
        if (target_y < 0) {
            if (this->_wrap_around_edges)
                target_y = height - 1;
            else
                return (1);
        }
    } else if (direction_moving == DIRECTION_RIGHT) {
        target_x = head.x + 1;
        if (target_x >= width) {
            if (this->_wrap_around_edges)
                target_x = 0;
            else
                return (1);
        }
    } else if (direction_moving == DIRECTION_DOWN) {
        target_y = head.y + 1; // DOWN increases Y coordinate
        if (target_y >= height) {
            if (this->_wrap_around_edges)
                target_y = 0;
            else
                return (1);
        }
    } else if (direction_moving == DIRECTION_LEFT) {
        target_x = head.x - 1;
        if (target_x < 0) {
            if (this->_wrap_around_edges)
                target_x = width - 1;
            else
                return (1);
        }
    }

    int target_val = this->_map.get(target_x, target_y, 2);
    if (target_val != 0 && target_val != FOOD && target_val != FIRE_FOOD && target_val != FROSTY_FOOD)
        return (1);
    if (this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL)
        return (1);

    return (0);
}

// Testing method - exposes private is_valid_move for unit tests
int game_data::test_is_valid_move(int player_head) {
    return (this->is_valid_move(player_head));
}

t_coordinates game_data::get_next_piece(t_coordinates current_coordinate, int piece_id) {
    int next_id = piece_id + 1;
    size_t width = this->_map.get_width();
    size_t height = this->_map.get_height();

    const int dirs[4][2] = {
        {0, 1},
        {1, 0},
        {0, -1},
        {-1, 0}};

    int i = 0;
    while (i < 4) {
        int nx = current_coordinate.x + dirs[i][0];
        int ny = current_coordinate.y + dirs[i][1];

        if (nx < 0) {
            if (this->_wrap_around_edges)
                nx = static_cast<int>(width) - 1;
            else {
                ++i;
                continue;
            }
        }
        if (nx >= static_cast<int>(width)) {
            if (this->_wrap_around_edges)
                nx = 0;
            else {
                ++i;
                continue;
            }
        }
        if (ny < 0) {
            if (this->_wrap_around_edges)
                ny = static_cast<int>(height) - 1;
            else {
                ++i;
                continue;
            }
        }
        if (ny >= static_cast<int>(height)) {
            if (this->_wrap_around_edges)
                ny = 0;
            else {
                ++i;
                continue;
            }
        }

        if (this->_map.get(static_cast<size_t>(nx),
                           static_cast<size_t>(ny), 2) == next_id)
            return ((t_coordinates){nx, ny});
        ++i;
    }

    return ((t_coordinates){-1, -1});
}

int game_data::update_snake_position(int player_head) {
    t_coordinates current_coords = this->get_head_coordinate(player_head);
    int player_number = this->determine_player_number(player_head);

    // Check if no direction is set - if so, don't move but don't game over either
    if (this->_direction_moving[player_number] == DIRECTION_NONE)
        return (0);

    // Check for collisions before moving
    if (this->is_valid_move(player_head) != 0)
        return (1);
    int direction_moving = this->_direction_moving[player_number];
    bool frosty_active = (this->_frosty_steps[player_number] > 0);
    if (frosty_active ||
        this->_map.get(current_coords.x, current_coords.y, 0) == GAME_TILE_ICE)
        direction_moving = this->_direction_moving_ice[player_number];
    int width = static_cast<int>(this->_map.get_width());
    int height = static_cast<int>(this->_map.get_height());
    int target_x = current_coords.x;
    int target_y = current_coords.y;
    if (direction_moving == DIRECTION_UP) {
        target_y = current_coords.y - 1; // UP decreases Y coordinate
        if (target_y < 0) {
            if (this->_wrap_around_edges)
                target_y = height - 1;
            else
                return (1); // Game over - hit boundary
        }
    } else if (direction_moving == DIRECTION_RIGHT) {
        target_x = current_coords.x + 1;
        if (target_x >= width) {
            if (this->_wrap_around_edges)
                target_x = 0;
            else
                return (1); // Game over - hit boundary
        }
    } else if (direction_moving == DIRECTION_DOWN) {
        target_y = current_coords.y + 1; // DOWN increases Y coordinate
        if (target_y >= height) {
            if (this->_wrap_around_edges)
                target_y = 0;
            else
                return (1); // Game over - hit boundary
        }
    } else if (direction_moving == DIRECTION_LEFT) {
        target_x = current_coords.x - 1;
        if (target_x < 0) {
            if (this->_wrap_around_edges)
                target_x = width - 1;
            else
                return (1); // Game over - hit boundary
        }
    }

    // Final safety check - ensure target coordinates are valid
    if (target_x < 0 || target_x >= width || target_y < 0 || target_y >= height)
        return (1); // Game over - invalid coordinates

    bool on_ice_now = (this->_map.get(current_coords.x, current_coords.y, 0) ==
                       GAME_TILE_ICE);
    bool on_ice_next = (this->_map.get(target_x, target_y, 0) == GAME_TILE_ICE);
    bool on_fire_next = (this->_map.get(target_x, target_y, 0) == GAME_TILE_FIRE);
    if (!on_ice_now && on_ice_next)
        this->_direction_moving_ice[player_number] = direction_moving;
    else if (on_ice_now && !on_ice_next && this->_frosty_steps[player_number] == 0)
        this->_direction_moving_ice[player_number] = 0;
    if (frosty_active)
    {
        this->_direction_moving_ice[player_number] = direction_moving;
        this->_frosty_steps[player_number]--;
        if (this->_frosty_steps[player_number] == 0 && !on_ice_next)
            this->_direction_moving_ice[player_number] = 0;
    }
    int offset = (player_head / 1000000) * 1000000;
    int tile_val = this->_map.get(target_x, target_y, 2);
    bool ate_food = (tile_val == FOOD || tile_val == FIRE_FOOD || tile_val == FROSTY_FOOD);

    // Move all existing segments forward by incrementing their values first
    // We need to do this in reverse order to avoid overwriting
    // IMPORTANT: Include the head (offset + 1) in the increment to avoid duplication
    for (int segment_value = offset + this->_snake_length[player_number]; segment_value >= offset + 1; segment_value--) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (this->_map.get(x, y, 2) == segment_value) {
                    this->_map.set(x, y, 2, segment_value + 1);
                }
            }
        }
    }

    // Now place new head at target position
    this->remove_empty_cell(target_x, target_y);
    this->_map.set(target_x, target_y, 2, offset + 1);

    // Remove the tail if no food was eaten
    if (!ate_food) {
        // Find and remove the tail segment (now has the highest value)
        int tail_value = offset + this->_snake_length[player_number] + 1;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (this->_map.get(x, y, 2) == tail_value) {
                    this->_map.set(x, y, 2, 0);
                    this->add_empty_cell(x, y);
                    break;
                }
            }
        }
    }

    // Handle food consumption
    if (ate_food) {
        if (tile_val == FIRE_FOOD)
            this->_speed_boost_steps[player_number] += 3;
        else if (tile_val == FROSTY_FOOD)
        {
            this->_frosty_steps[player_number] = 3;
            this->_direction_moving_ice[player_number] = direction_moving;
        }
        ft_achievement &apple =
            this->_character.get_achievements().at(ACH_APPLES_EATEN);
        int progress = apple.get_progress(ACH_GOAL_PRIMARY);
        if (progress < INT_MAX)
            apple.set_progress(ACH_GOAL_PRIMARY, progress + 1);
        if (this->_snake_length[player_number] < MAX_SNAKE_LENGTH) {
            this->_snake_length[player_number]++;
            ft_achievement &snake =
                this->_character.get_achievements().at(ACH_SNAKE_50);
            int goal = snake.get_goal(ACH_GOAL_PRIMARY);
            if (this->_snake_length[player_number] >= goal &&
                !snake.is_goal_complete(ACH_GOAL_PRIMARY))
                snake.set_progress(ACH_GOAL_PRIMARY, goal);
            this->spawn_food();
        }
    }
    if (on_fire_next)
    {
        this->_speed_boost_steps[player_number] += 3;
        this->_map.set(target_x, target_y, 0, GAME_TILE_EMPTY);
        if (this->_additional_food_items)
            this->spawn_fire_tile();
    }
    return (0);
}

int game_data::update_game_map(double deltaTime)
{
    int ret = 0;
    int heads[4] = {
        SNAKE_HEAD_PLAYER_1,
        SNAKE_HEAD_PLAYER_2,
        SNAKE_HEAD_PLAYER_3,
        SNAKE_HEAD_PLAYER_4};
    const double baseInterval = 1.0 / this->_moves_per_second; // Moves per second is configurable
    int i = 0;
    while (i < 4)
    {
        if (this->_snake_length[i] > 0)
        {
            this->_update_timer[i] += deltaTime;
            double interval = baseInterval;
            if (this->_speed_boost_steps[i] > 0)
                interval /= 1.5;
            while (this->_update_timer[i] >= interval)
            {
                this->_update_timer[i] -= interval;
                bool boosted = (this->_speed_boost_steps[i] > 0);
                if (this->update_snake_position(heads[i]))
                    ret = 1;
                if (boosted && this->_speed_boost_steps[i] > 0)
                    this->_speed_boost_steps[i]--;
                interval = baseInterval;
                if (this->_speed_boost_steps[i] > 0)
                    interval /= 1.5;
            }
        }
        ++i;
    }
    return (ret);
}
