#include "libft/Game/character.hpp"
#include "libft/Game/map3d.hpp"

#define GAME_TILE_EMPTY 0
#define GAME_TILE_WALL 1
#define GAME_TILE_ICE 2

#define SNAKE_HEAD_PLAYER_1 1000001
#define SNAKE_HEAD_PLAYER_2 2000001
#define SNAKE_HEAD_PLAYER_3 3000001
#define SNAKE_HEAD_PLAYER_4 4000001

#define DIRECTION_UP 0
#define DIRECTION_RIGHT 1
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 3

#define SUCCES 0
#define FAILURE 1

static const int MAX_SNAKE_LENGTH = 40000;

typedef struct s_coordinates
{
	int x;
	int y;
} t_coordinates;

class game_data
{
        public:
                game_data(int width, int height);
		mutable int		_error;
		int				_wrap_around_edges;
		int				_amount_players_dead;
		int				_direction_moving[4];
		int				_direction_moving_ice[4];

		ft_map3d		_map;
		ft_character	_character;

		t_coordinates 	get_head_coordinate(int head_to_find);
		int				is_valid_move(int player_head);
		int				update_snake_position(int player_head);
		int				determine_player_number(int player_head);

		t_coordinates	get_next_piece(t_coordinates current_coordinate, int piece_id);

	private:
		int	update_game_map();
};
