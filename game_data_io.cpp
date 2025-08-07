#include "game_data.hpp"
#include "libft/JSon/json.hpp"
#include <filesystem>
#include <cstdlib>
#include <string>
#include <cstring>

static std::filesystem::path get_save_dir()
{
    return (std::filesystem::current_path() / "save_data");
}

static void ensure_save_dir_exists()
{
    std::filesystem::path dir = get_save_dir();
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }
    return ;
}

game_data::game_data(int width, int height) :
        _error(0), _wrap_around_edges(0), _amount_players_dead(0),
        _profile_name("default"), _achievement_snake50(false),
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
                this->_direction_moving[index] = DIRECTION_NONE;
                // Only initialize Player 1 snake, others are inactive (length 0)
                this->_snake_length[index] = (index == 0) ? 4 : 0;
                this->_update_counter[index] = 0;
                index++;
        }
        ensure_save_dir_exists();
        this->reset_board();
        return ;
}

int game_data::save_game() const
{
    ensure_save_dir_exists();
    std::filesystem::path file = get_save_dir() /
                (std::string(this->_profile_name.c_str()) + ".json");
    json_group *group = json_create_json_group("game");
    if (!group)
        return (1);
    json_add_item_to_group(group, json_create_item
                        ("snake_length", this->_snake_length[0]));
    json_add_item_to_group(group, json_create_item
                        ("achievement_snake50", this->_achievement_snake50));
    int ret = json_write_to_file(file.c_str(), group);
    json_free_groups(group);
    return (ret);
}

int game_data::load_game()
{
    ensure_save_dir_exists();
    std::filesystem::path file = get_save_dir() /
                (std::string(this->_profile_name.c_str()) + ".json");
    json_group *root = json_read_from_file(file.c_str());
    if (!root)
        return (1);
    json_group *group = json_find_group(root, "game");
    if (!group)
    {
        json_free_groups(root);
        return (1);
    }
    json_item *len = json_find_item(group, "snake_length");
    if (len)
        this->_snake_length[0] = std::atoi(len->value);
    json_item *ach = json_find_item(group, "achievement_snake50");
    if (ach)
        this->_achievement_snake50 = (std::strcmp(ach->value, "true")
                                == 0 || std::strcmp(ach->value, "1") == 0);
    json_free_groups(root);
    return (0);
}

