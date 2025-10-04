#include "game_data.hpp"
int game_data::get_error() const {
    return (this->_error);
}

void game_data::set_wrap_around_edges(int value) {
    this->_wrap_around_edges = value;
    return;
}

int game_data::get_wrap_around_edges() const {
    return (this->_wrap_around_edges);
}

void game_data::set_moves_per_second(double moves)
{
    this->_moves_per_second = moves;
    return ;
}

double game_data::get_moves_per_second() const
{
    return (this->_moves_per_second);
}

void game_data::set_additional_food_items(int value)
{
    this->_additional_food_items = value;
}

int game_data::get_additional_food_items() const
{
    return (this->_additional_food_items);
}


void game_data::set_direction_moving(int player, int direction)
{
    if (player >= 0 && player < 4)
    {
        int idx = player;

        // Determine if the requested direction would move into the second
        // segment of the snake. If so, ignore the input to prevent the snake
        // from reversing into itself.
        if (this->_snake_length[idx] > 1) {
            int head_ids[4] = {SNAKE_HEAD_PLAYER_1, SNAKE_HEAD_PLAYER_2,
                               SNAKE_HEAD_PLAYER_3, SNAKE_HEAD_PLAYER_4};
            t_coordinates head = this->get_head_coordinate(head_ids[idx]);
            t_coordinates second = this->get_next_piece(head, head_ids[idx]);

            if (second.x != -1 && second.y != -1) {
                int blocked_dir = DIRECTION_NONE;

                if (this->_wrap_around_edges) {
                    int width = static_cast<int>(this->_map.get_width());
                    int height = static_cast<int>(this->_map.get_height());

                    if (second.x == (head.x - 1 + width) % width && second.y == head.y)
                        blocked_dir = DIRECTION_LEFT;
                    else if (second.x == (head.x + 1) % width && second.y == head.y)
                        blocked_dir = DIRECTION_RIGHT;
                    else if (second.y == (head.y - 1 + height) % height && second.x == head.x)
                        blocked_dir = DIRECTION_UP;
                    else if (second.y == (head.y + 1) % height && second.x == head.x)
                        blocked_dir = DIRECTION_DOWN;
                } else {
                    if (second.x == head.x - 1 && second.y == head.y)
                        blocked_dir = DIRECTION_LEFT;
                    else if (second.x == head.x + 1 && second.y == head.y)
                        blocked_dir = DIRECTION_RIGHT;
                    else if (second.y == head.y - 1 && second.x == head.x)
                        blocked_dir = DIRECTION_UP;
                    else if (second.y == head.y + 1 && second.x == head.x)
                        blocked_dir = DIRECTION_DOWN;
                }

                if (direction == blocked_dir)
                    return;
            }
        }

        this->_direction_moving[idx] = direction;
    }
    return;
}

int game_data::get_direction_moving(int player) const {
    if (player >= 0 && player < 4)
        return (this->_direction_moving[player]);
    return (DIRECTION_NONE);
}

void game_data::set_profile_name(const ft_string& name) {
    this->_profile_name = name;
	if (this->_profile_name.get_error())
		this->_error = this->_profile_name.get_error();
    return ;
}

const ft_string& game_data::get_profile_name() const {
    return (this->_profile_name);
}

void game_data::set_map_name(const char *name) {
    if (!name) {
        this->_map_name.clear();
        return;
    }
    this->_map_name = name;
}

const char *game_data::get_map_name() const {
    return this->_map_name.c_str();
}

int game_data::get_snake_length(int player) const {
    if (player >= 0 && player < 4)
        return this->_snake_length[player];
    return (0);
}

bool game_data::get_achievement_snake50() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_SNAKE_50);
    if (!ach)
        return false;
    return ach->value.is_goal_complete(ACH_GOAL_PRIMARY);
}

int game_data::get_apples_eaten() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_APPLES_EATEN);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}

int game_data::get_apples_normal_eaten() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_APPLES_NORMAL_EATEN);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}

int game_data::get_apples_frosty_eaten() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_APPLES_FROSTY_EATEN);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}

int game_data::get_apples_fire_eaten() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_APPLES_FIRE_EATEN);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}

int game_data::get_tile_normal_steps() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_TILE_NORMAL_STEPS);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}

int game_data::get_tile_frosty_steps() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_TILE_FROSTY_STEPS);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}

int game_data::get_tile_fire_steps() const
{
    const Pair<int, ft_achievement> *ach =
        this->_character.get_achievements().find(ACH_TILE_FIRE_STEPS);
    if (!ach)
        return 0;
    return ach->value.get_progress(ACH_GOAL_PRIMARY);
}
