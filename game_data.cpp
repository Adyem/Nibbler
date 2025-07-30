#include "game_data.hpp"
#include "libft/Game/character.hpp"
#include "libft/Game/map3d.hpp"

game_data::game_data(int width, int height) :
	_error(0), _wrap_around_edges(0), _amount_players_dead(0),
	_map(width, height, 2), _character()
{
	if (this->_map.get_error())
		this->_error = this->_map.get_error();
	else if (this->_character.get_error())
		this->_error = this->_character.get_error();
	int index = 0;
	while (index < 4)
	{
		this->_direction_moving_ice[index] = 0;
		this->_direction_moving[index] = 0;
		index++;
	}
	return ;
}

t_coordinates game_data::get_head_coordinate(int head_to_find)
{
	int index_y = 0;
	while (index_y < this->_map.get_height())
	{
		int index_x = 0;
		while (index_x < this->_map.get_width())
		{
			if (this->_map.get(index_x, index_y, 2) == head_to_find)
				return ((t_coordinates){index_x, index_y});
			index_x++;
		}
		index_y++;
	}
	return ((t_coordinates){0, 0});
}

int game_data::determine_player_number(int player_head)
{
	return ((player_head % 1000000) - 1);
}

int game_data::is_valid_move(int player_head)
{
    t_coordinates head = this->get_head_coordinate(player_head);
    int player_number = determine_player_number(player_head);
	int direction_moving = this->_direction_moving[player_number];
	if (this->_map.get(head.x, head.y, 0) == GAME_TILE_ICE)
		direction_moving = this->_direction_moving_ice[player_number];

    int width  = this->_map.get_width();
    int height = this->_map.get_height();

    if (head.x < 0 || head.x >= width || head.y < 0 || head.y >= height)
        return (1);

    int target_x = head.x;
    int target_y = head.y;

    if (direction_moving == DIRECTION_UP)
    {
        target_y = head.y + 1;
        if (target_y >= height)
        {
            if (this->_wrap_around_edges)
                target_y = 0;
            else
                return (1);
        }
    }
    else if (direction_moving == DIRECTION_RIGHT)
    {
        target_x = head.x + 1;
        if (target_x >= width)
        {
            if (this->_wrap_around_edges)
                target_x = 0;
            else
                return (1);
        }
    }
    else if (direction_moving == DIRECTION_DOWN)
    {
        target_y = head.y - 1;
        if (target_y < 0)
        {
            if (this->_wrap_around_edges)
                target_y = height - 1;
            else
                return (1);
        }
    }
    else if (direction_moving == DIRECTION_LEFT)
    {
        target_x = head.x - 1;
        if (target_x < 0)
        {
            if (this->_wrap_around_edges)
                target_x = width - 1;
            else
                return (1);
        }
    }
    if (this->_map.get(target_x, target_y, 2) != 0)
        return (1);

    return (0);
}

t_coordinates game_data::get_next_piece(t_coordinates current_coordinate, int piece_id)
{
	t_coordinates next_piece;

	next_piece.x = -1;
	next_piece.y = -1;
	return (next_piece);
}

int	game_data::update_snake_position(int player_head)
{
	t_coordinates current_coords = get_head_coordinate(player_head);
	int player_number = determine_player_number(player_head);

	int return_value = 0;
	if (is_valid_move(player_head))
		return_value = 1;
	int direction_moving = this->_direction_moving[player_number];
	if (this->_map.get(current_coords.x, current_coords.y, 0) == GAME_TILE_ICE)
		direction_moving = this->_direction_moving_ice[player_number];
	return (return_value);
}

int game_data::update_game_map()
{
	return (update_snake_position(SNAKE_HEAD_PLAYER_1));
}
