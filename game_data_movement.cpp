#include "game_data.hpp"
#include <limits>

namespace
{
constexpr int FIRE_BOOST_STEPS = 5;
}

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
    return ((player_head / 1000000) - 1);
}

bool game_data::advance_wrap_target(int direction, int &target_x, int &target_y) const {
    int width = static_cast<int>(this->_map.get_width());
    int height = static_cast<int>(this->_map.get_height());
    int limit = (direction == DIRECTION_UP || direction == DIRECTION_DOWN) ? height : width;

    for (int step = 0; step < limit; ++step) {
        if (this->_map.get(target_x, target_y, 0) != GAME_TILE_WALL)
            return true;

        switch (direction) {
        case DIRECTION_UP:
            target_y = (target_y - 1 + height) % height;
            break;
        case DIRECTION_DOWN:
            target_y = (target_y + 1) % height;
            break;
        case DIRECTION_LEFT:
            target_x = (target_x - 1 + width) % width;
            break;
        case DIRECTION_RIGHT:
            target_x = (target_x + 1) % width;
            break;
        default:
            return false;
        }
    }
    return false;
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
            if (!this->_wrap_around_edges)
                return (1);
            target_y = height - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_y == 0 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_y = height - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    } else if (direction_moving == DIRECTION_RIGHT) {
        target_x = head.x + 1;
        if (target_x >= width) {
            if (!this->_wrap_around_edges)
                return (1);
            target_x = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_x == width - 1 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_x = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    } else if (direction_moving == DIRECTION_DOWN) {
        target_y = head.y + 1; // DOWN increases Y coordinate
        if (target_y >= height) {
            if (!this->_wrap_around_edges)
                return (1);
            target_y = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_y == height - 1 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_y = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    } else if (direction_moving == DIRECTION_LEFT) {
        target_x = head.x - 1;
        if (target_x < 0) {
            if (!this->_wrap_around_edges)
                return (1);
            target_x = width - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_x == 0 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_x = width - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    }

    int target_val = this->_map.get(target_x, target_y, 2);
    int offset = (player_number + 1) * 1000000;
    int tail_value = offset + this->_snake_length[player_number];
    if (target_val != 0 && target_val != FOOD && target_val != FIRE_FOOD && target_val != FROSTY_FOOD && target_val != tail_value)
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
            if (!this->_wrap_around_edges)
                return (1); // Game over - hit boundary
            target_y = height - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_y == 0 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_y = height - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    } else if (direction_moving == DIRECTION_RIGHT) {
        target_x = current_coords.x + 1;
        if (target_x >= width) {
            if (!this->_wrap_around_edges)
                return (1); // Game over - hit boundary
            target_x = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_x == width - 1 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_x = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    } else if (direction_moving == DIRECTION_DOWN) {
        target_y = current_coords.y + 1; // DOWN increases Y coordinate
        if (target_y >= height) {
            if (!this->_wrap_around_edges)
                return (1); // Game over - hit boundary
            target_y = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_y == height - 1 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_y = 0;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    } else if (direction_moving == DIRECTION_LEFT) {
        target_x = current_coords.x - 1;
        if (target_x < 0) {
            if (!this->_wrap_around_edges)
                return (1); // Game over - hit boundary
            target_x = width - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        } else if (this->_wrap_around_edges && target_x == 0 &&
                   this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL) {
            target_x = width - 1;
            if (!this->advance_wrap_target(direction_moving, target_x, target_y))
                return (1);
        }
    }

    // Final safety check - ensure target coordinates are valid
    if (target_x < 0 || target_x >= width || target_y < 0 || target_y >= height)
        return (1); // Game over - invalid coordinates

    bool on_ice_now = (this->_map.get(current_coords.x, current_coords.y, 0) ==
                       GAME_TILE_ICE);
    int tile_type = this->_map.get(target_x, target_y, 0);
    bool on_ice_next = (tile_type == GAME_TILE_ICE);
    bool on_fire_next = (tile_type == GAME_TILE_FIRE);
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
    int offset = (player_number + 1) * 1000000;
    int tile_val = this->_map.get(target_x, target_y, 2);
    bool ate_food = (tile_val == FOOD || tile_val == FIRE_FOOD || tile_val == FROSTY_FOOD);
    bool target_is_tail = (tile_val == offset + this->_snake_length[player_number]);

    // When stepping onto the current tail, remove that tail segment first so
    // the new head can occupy its cell without overwriting data that still
    // needs to be shifted forward.
    if (target_is_tail && !ate_food)
    {
        this->_map.set(target_x, target_y, 2, 0);
    }

    // Move existing segments forward by incrementing their values. If the
    // target is the tail, we can skip that tile since it was cleared above.
    int start_value = offset + this->_snake_length[player_number] -
                      (target_is_tail && !ate_food ? 1 : 0);
    for (int segment_value = start_value; segment_value >= offset + 1; --segment_value)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (this->_map.get(x, y, 2) == segment_value)
                    this->_map.set(x, y, 2, segment_value + 1);
            }
        }
    }

    // Place new head at target position
    if (!target_is_tail || ate_food)
        this->remove_empty_cell(target_x, target_y);
    this->_map.set(target_x, target_y, 2, offset + 1);

    // Remove the tail if no food was eaten and we didn't move into it
    if (!ate_food && !target_is_tail)
    {
        int tail_value = offset + this->_snake_length[player_number] + 1;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (this->_map.get(x, y, 2) == tail_value)
                {
                    this->_map.set(x, y, 2, 0);
                    this->add_empty_cell(x, y);
                    break;
                }
            }
        }
    }

    // Update tile stepping achievements
    {
        ft_achievement *tile_ach = nullptr;
        if (tile_type == GAME_TILE_FIRE)
            tile_ach = &this->_character.get_achievements().at(ACH_TILE_FIRE_STEPS);
        else if (tile_type == GAME_TILE_ICE)
            tile_ach = &this->_character.get_achievements().at(ACH_TILE_FROSTY_STEPS);
        else
            tile_ach = &this->_character.get_achievements().at(ACH_TILE_NORMAL_STEPS);
        int progress = tile_ach->get_progress(ACH_GOAL_PRIMARY);
        if (progress < std::numeric_limits<int>::max())
            tile_ach->set_progress(ACH_GOAL_PRIMARY, progress + 1);
    }

    // Handle food consumption
    if (ate_food) {
        if (tile_val == FIRE_FOOD)
        {
            this->_fire_boost_active[player_number] = true;
            this->_speed_boost_steps[player_number] = FIRE_BOOST_STEPS;
            ft_achievement &fire =
                this->_character.get_achievements().at(ACH_APPLES_FIRE_EATEN);
            int progress_fire = fire.get_progress(ACH_GOAL_PRIMARY);
            if (progress_fire < std::numeric_limits<int>::max())
                fire.set_progress(ACH_GOAL_PRIMARY, progress_fire + 1);
        }
        else if (tile_val == FROSTY_FOOD)
        {
            this->_frosty_steps[player_number] = 3;
            this->_direction_moving_ice[player_number] = direction_moving;
            ft_achievement &frost =
                this->_character.get_achievements().at(ACH_APPLES_FROSTY_EATEN);
            int progress_frost = frost.get_progress(ACH_GOAL_PRIMARY);
            if (progress_frost < std::numeric_limits<int>::max())
                frost.set_progress(ACH_GOAL_PRIMARY, progress_frost + 1);
        }
        else
        {
            ft_achievement &normal =
                this->_character.get_achievements().at(ACH_APPLES_NORMAL_EATEN);
            int progress_norm = normal.get_progress(ACH_GOAL_PRIMARY);
            if (progress_norm < std::numeric_limits<int>::max())
                normal.set_progress(ACH_GOAL_PRIMARY, progress_norm + 1);
        }
        ft_achievement &apple =
            this->_character.get_achievements().at(ACH_APPLES_EATEN);
        int progress = apple.get_progress(ACH_GOAL_PRIMARY);
        if (progress < std::numeric_limits<int>::max())
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
        this->_fire_boost_active[player_number] = true;
        this->_speed_boost_steps[player_number] = FIRE_BOOST_STEPS;
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
            if (this->_fire_boost_active[i] || this->_speed_boost_steps[i] > 0)
                interval /= 1.5;
            while (this->_update_timer[i] >= interval)
            {
                this->_update_timer[i] -= interval;
                if (this->update_snake_position(heads[i]))
                    ret = 1;
                if (this->_speed_boost_steps[i] > 0)
                {
                    --this->_speed_boost_steps[i];
                    if (this->_speed_boost_steps[i] <= 0)
                    {
                        this->_speed_boost_steps[i] = 0;
                        this->_fire_boost_active[i] = false;
                    }
                }
                interval = baseInterval;
                if (this->_fire_boost_active[i] || this->_speed_boost_steps[i] > 0)
                    interval /= 1.5;
            }
        }
        ++i;
    }
    return (ret);
}
