#include "libft/Game/character.hpp"
#include "libft/Game/map3d.hpp"
#include "libft/CPP_class/string_class.hpp"

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
#define FOOD 1

typedef struct s_coordinates
{
    int x;
    int y;
} t_coordinates;

class game_data
{
        public:
        game_data(int width, int height);
        void reset_board();
    	void resize_board(int width, int height);

    	int  get_error() const;
    	void set_wrap_around_edges(int value);
    	int  get_wrap_around_edges() const;
    	void set_direction_moving(int player, int direction);
    	int  get_direction_moving(int player) const;

    	void   set_map_value(int x, int y, int layer, int value);
    	int    get_map_value(int x, int y, int layer) const;
    	size_t get_width() const;
    	size_t get_height() const;

    	void spawn_food();

        t_coordinates get_head_coordinate(int head_to_find);
        int           is_valid_move(int player_head);
        int           update_snake_position(int player_head);
        int           update_game_map();

        void        set_profile_name(const ft_string &name);
        const ft_string &get_profile_name() const;
        int         save_game() const;
        int         load_game();
        int         get_snake_length(int player) const;
        bool        get_achievement_snake50() const;

        private:
        t_coordinates get_next_piece(t_coordinates current_coordinate, int piece_id);
        int           determine_player_number(int player_head);

    	mutable int _error;
    	int         _wrap_around_edges;
    	int         _amount_players_dead;
        int         _direction_moving[4];
        int         _direction_moving_ice[4];
        int         _snake_length[4];
        ft_string   _profile_name;
        bool        _achievement_snake50;

        ft_map3d     _map;
        ft_character _character;
};
