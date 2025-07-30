#include "game_data.hpp"
#include "libft/Game/character.hpp"
#include "libft/Game/map3d.hpp"

game_data::game_data(int width, int height) :
	_error(0), _wrap_around_edges(0), _amount_players_dead(0),
        _map(width, height, 3), _character()
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
        size_t index_y = 0;
        while (index_y < this->_map.get_height())
        {
                size_t index_x = 0;
                while (index_x < this->_map.get_width())
                {
                        if (this->_map.get(index_x, index_y, 2) == head_to_find)
                                return ((t_coordinates){static_cast<int>(index_x), static_cast<int>(index_y)});
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

    int width  = static_cast<int>(this->_map.get_width());
    int height = static_cast<int>(this->_map.get_height());

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
    if (this->_map.get(target_x, target_y, 0) == GAME_TILE_WALL)
        return (1);

    return (0);
}

t_coordinates game_data::get_next_piece(t_coordinates current_coordinate, int piece_id)
{
    int next_id = piece_id + 1;
    size_t width = this->_map.get_width();
    size_t height = this->_map.get_height();

    const int dirs[4][2] = {
        {0, 1},  // up
        {1, 0},  // right
        {0, -1}, // down
        {-1, 0}  // left
    };

    for (int i = 0; i < 4; i++)
    {
        int nx = current_coordinate.x + dirs[i][0];
        int ny = current_coordinate.y + dirs[i][1];

        if (nx < 0)
        {
            if (this->_wrap_around_edges)
                nx = static_cast<int>(width) - 1;
            else
                continue;
        }
        if (nx >= static_cast<int>(width))
        {
            if (this->_wrap_around_edges)
                nx = 0;
            else
                continue;
        }
        if (ny < 0)
        {
            if (this->_wrap_around_edges)
                ny = static_cast<int>(height) - 1;
            else
                continue;
        }
        if (ny >= static_cast<int>(height))
        {
            if (this->_wrap_around_edges)
                ny = 0;
            else
                continue;
        }

        if (this->_map.get(static_cast<size_t>(nx), static_cast<size_t>(ny), 2) == next_id)
            return ((t_coordinates){nx, ny});
    }

    return ((t_coordinates){-1, -1});
}


int game_data::update_game_map()
{
	return (update_snake_position(SNAKE_HEAD_PLAYER_1));
}

int     game_data::update_snake_position(int player_head)
{
    t_coordinates current_coords = get_head_coordinate(player_head);
    int player_number = determine_player_number(player_head);

    if (is_valid_move(player_head))
        return (1);
    int direction_moving = this->_direction_moving[player_number];
    if (this->_map.get(current_coords.x, current_coords.y, 0) == GAME_TILE_ICE)
        direction_moving = this->_direction_moving_ice[player_number];
    int width  = static_cast<int>(this->_map.get_width());
    int height = static_cast<int>(this->_map.get_height());
    int target_x = current_coords.x;
    int target_y = current_coords.y;
    if (direction_moving == DIRECTION_UP)
    {
        target_y = current_coords.y + 1;
        if (target_y >= height)
            target_y = (this->_wrap_around_edges ? 0 : height);
    }
    else if (direction_moving == DIRECTION_RIGHT)
    {
        target_x = current_coords.x + 1;
        if (target_x >= width)
            target_x = (this->_wrap_around_edges ? 0 : width);
    }
    else if (direction_moving == DIRECTION_DOWN)
    {
        target_y = current_coords.y - 1;
        if (target_y < 0)
            target_y = (this->_wrap_around_edges ? height - 1 : -1);
    }
    else if (direction_moving == DIRECTION_LEFT)
    {
        target_x = current_coords.x - 1;
        if (target_x < 0)
            target_x = (this->_wrap_around_edges ? width - 1 : -1);
    }
    bool on_ice_now = (this->_map.get(current_coords.x, current_coords.y, 0) ==
		GAME_TILE_ICE);
    bool on_ice_next = (this->_map.get(target_x, target_y, 0) == GAME_TILE_ICE);
    if (!on_ice_now && on_ice_next)
        this->_direction_moving_ice[player_number] = direction_moving;
    else if (on_ice_now && !on_ice_next)
        this->_direction_moving_ice[player_number] = 0;
    int offset = (player_head / 1000000) * 1000000;
    int y = 0;
    while (y < height)
    {
        int x = 0;
        while (x < width)
        {
            int val = this->_map.get(x, y, 2);
            if (val >= offset + 1 && val < offset + MAX_SNAKE_LENGTH)
                this->_map.set(x, y, 2, val + 1);
            else if (val >= offset + MAX_SNAKE_LENGTH)
                this->_map.set(x, y, 2, 0);
            x++;
        }
        y++;
    }
    this->_map.set(target_x, target_y, 2, offset + 1);
    return (0);
}

