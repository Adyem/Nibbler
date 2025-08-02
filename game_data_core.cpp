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
        this->_direction_moving[player] = direction;
    }
    return ;
}

int game_data::get_direction_moving(int player) const
{
    if (player >= 0 && player < 4)
        return (this->_direction_moving[player]);
    return (0);
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

