#ifndef GAME_DATA_HPP
#define GAME_DATA_HPP

#include "libft/Game/character.hpp"
#include "libft/Game/map3d.hpp"
#include "libft/CPP_class/string_class.hpp"
#include <vector>
#include <string>
#include <deque>

#define GAME_TILE_EMPTY 0
#define GAME_TILE_WALL 1
#define GAME_TILE_ICE 2
#define GAME_TILE_FIRE 3

enum : char {
    MAP_TILE_EMPTY       = '0',
    MAP_TILE_WALL        = '1',
    MAP_TILE_ICE         = '2',
    MAP_TILE_FIRE        = '3',
    MAP_TILE_SNAKE_HEAD  = '4',
    MAP_TILE_SNAKE_BODY_1 = '5',
    MAP_TILE_SNAKE_BODY_2 = '6',
    MAP_TILE_SNAKE_BODY_3 = '7',
};

#define SNAKE_HEAD_PLAYER_1 1000001
#define SNAKE_HEAD_PLAYER_2 2000001
#define SNAKE_HEAD_PLAYER_3 3000001
#define SNAKE_HEAD_PLAYER_4 4000001

#define DIRECTION_NONE -1
#define DIRECTION_UP 0
#define DIRECTION_RIGHT 1
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 3

#define SUCCES 0
#define FAILURE 1

static const int MAX_SNAKE_LENGTH = 40000;
#define FOOD 1
#define FIRE_FOOD 2
#define FROSTY_FOOD 3

#define ACH_APPLES_EATEN 0
#define ACH_SNAKE_50 1
#define ACH_APPLES_NORMAL_EATEN 2
#define ACH_APPLES_FROSTY_EATEN 3
#define ACH_APPLES_FIRE_EATEN 4
#define ACH_TILE_NORMAL_STEPS 5
#define ACH_TILE_FROSTY_STEPS 6
#define ACH_TILE_FIRE_STEPS 7
#define ACH_GOAL_PRIMARY 0

typedef struct s_coordinates {
    int x;
    int y;
} t_coordinates;

class game_data
{
    public:
        game_data(int width, int height);
        void reset_board();
        void resize_board(int width, int height);
        void spawn_food();

        int  get_error() const;
        void set_wrap_around_edges(int value);
        int  get_wrap_around_edges() const;
        void set_direction_moving(int player, int direction);
        int  get_direction_moving(int player) const;

        void set_moves_per_second(double moves);
        double get_moves_per_second() const;
        void set_additional_food_items(int value);
        int  get_additional_food_items() const;

        void   set_map_value(int x, int y, int layer, int value);
        int    get_map_value(int x, int y, int layer) const;
        size_t get_width() const;
        size_t get_height() const;

        void reset_player_status_effects(int player);

        t_coordinates get_head_coordinate(int head_to_find);

        int         update_game_map(double deltaTime);

        void        set_profile_name(const ft_string &name);
        const ft_string &get_profile_name() const;
        void        set_map_name(const char *name);
        const char *get_map_name() const;
        int         save_game() const;
        int         load_game();
        int         get_snake_length(int player) const;
        void        set_player_snake_length(int player, int length);
        bool        get_achievement_snake50() const;
        int         get_apples_eaten() const;
        int         get_apples_normal_eaten() const;
        int         get_apples_frosty_eaten() const;
        int         get_apples_fire_eaten() const;
        int         get_tile_normal_steps() const;
        int         get_tile_frosty_steps() const;
        int         get_tile_fire_steps() const;

        void        sync_snake_segments_from_map();

    // Testing method - exposes private is_valid_move for unit tests
    int test_is_valid_move(int player_head);

    private:
        t_coordinates get_next_piece(t_coordinates current_coordinate, int piece_id);
        int         determine_player_number(int player_head);
        void        add_empty_cell(int x, int y);
        void        remove_empty_cell(int x, int y);
        void        initialize_empty_cells();
        void        apply_snake_segments(int player, const std::vector<t_coordinates> &segments);
        void        rebuild_snake_segments_from_map(int player);
        void        write_snake_to_map(int player);

        int         is_valid_move(int player_head);
        bool        advance_wrap_target(int direction, int &target_x, int &target_y) const;
        int         update_snake_position(int player_head);
        void        spawn_fire_tile();
        bool        can_spawn_frosty_food(int x, int y);
        bool        is_tile_free(int &x, int &y) const;

        mutable int _error;
        int         _wrap_around_edges;
        int         _amount_players_dead;
        int         _direction_moving[4];
        int         _direction_moving_ice[4];
        int         _speed_boost_steps[4];
        bool        _fire_boost_active[4];
        int         _frosty_steps[4];
        int         _snake_length[4];
        double      _update_timer[4];
        double      _moves_per_second;
        int         _additional_food_items;
        ft_string   _profile_name;
        std::string _map_name;
        ft_map3d                                _map;
        ft_character                            _character;
        std::vector<t_coordinates>      _empty_cells;
        std::vector<int>                _empty_cell_indices;
        std::deque<t_coordinates>       _snake_segments[4];
};

#endif // GAME_DATA_HPP
