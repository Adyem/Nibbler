#include "game_data.hpp"

int game_data::get_error() const
{
    return (this->_error);
}

void game_data::set_wrap_around_edges(int value)
{
    this->_wrap_around_edges = value;
    return ;
}

int game_data::get_wrap_around_edges() const
{
    return (this->_wrap_around_edges);
}

void game_data::set_direction_moving(int player, int direction)
{
    if (player >= 0 && player < 4)
    {
        // Prevent moving backwards into the snake's own body
        int current_direction = this->_direction_moving[player];

        // Only prevent backwards movement if the snake is already moving
        if (current_direction != DIRECTION_NONE)
        {
            // Check if the new direction is opposite to the current direction
            bool is_opposite = false;
            if ((current_direction == DIRECTION_UP && direction == DIRECTION_DOWN) ||
                (current_direction == DIRECTION_DOWN && direction == DIRECTION_UP) ||
                (current_direction == DIRECTION_LEFT && direction == DIRECTION_RIGHT) ||
                (current_direction == DIRECTION_RIGHT && direction == DIRECTION_LEFT))
            {
                is_opposite = true;
            }

            // If it's not an opposite direction, allow the change
            if (!is_opposite)
            {
                this->_direction_moving[player] = direction;
            }
            // If it is opposite, ignore the input (don't change direction)
        }
        else
        {
            // If snake is not moving yet, allow any direction
            this->_direction_moving[player] = direction;
        }
    }
    return ;
}

int game_data::get_direction_moving(int player) const
{
    if (player >= 0 && player < 4)
        return (this->_direction_moving[player]);
    return (DIRECTION_NONE);
}

void game_data::set_profile_name(const ft_string &name)
{
    this->_profile_name = name;
    return ;
}

const ft_string &game_data::get_profile_name() const
{
    return (this->_profile_name);
}

int game_data::get_snake_length(int player) const
{
    if (player >= 0 && player < 4)
        return this->_snake_length[player];
    return (0);
}

bool game_data::get_achievement_snake50() const
{
    return (this->_achievement_snake50);
}
