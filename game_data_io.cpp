#include "game_data.hpp"
#include "libft/JSon/json.hpp"
#include <filesystem>
#include <cstdlib>
#include <string>
#include <cstring>
#include <climits>

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
        _moves_per_second(1.0), _additional_food_items(0),
        _profile_name("default"),
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
                this->_speed_boost_steps[index] = 0;
                this->_frosty_steps[index] = 0;
                // Only initialize Player 1 snake, others are inactive (length 0)
                this->_snake_length[index] = (index == 0) ? 4 : 0;
                this->_update_timer[index] = 0.0;
                index++;
        }
        ft_achievement apple;
        apple.set_id(ACH_APPLES_EATEN);
        apple.set_goal(ACH_GOAL_PRIMARY, INT_MAX);
        ft_map<int, ft_achievement> &ach = this->_character.get_achievements();
        ach.insert(ACH_APPLES_EATEN, apple);
        if (ach.get_error())
                this->_error = ach.get_error();
        ft_achievement snake50;
        snake50.set_id(ACH_SNAKE_50);
        snake50.set_goal(ACH_GOAL_PRIMARY, 50);
        ach.insert(ACH_SNAKE_50, snake50);
        if (ach.get_error())
                this->_error = ach.get_error();
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
    const ft_map<int, ft_achievement> &achievements =
        this->_character.get_achievements();
    const Pair<int, ft_achievement> *apple =
        achievements.find(ACH_APPLES_EATEN);
    int appleCount = apple ? apple->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *snake =
        achievements.find(ACH_SNAKE_50);
    bool snake50 = snake ?
        snake->value.is_goal_complete(ACH_GOAL_PRIMARY) : false;
    json_add_item_to_group(group, json_create_item
                        ("achievement_snake50", snake50));
    json_add_item_to_group(group, json_create_item
                        ("apples_eaten", appleCount));
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
    {
        bool unlocked = (std::strcmp(ach->value, "true") == 0
                        || std::strcmp(ach->value, "1") == 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_SNAKE_50);
        if (unlocked)
            a.set_progress(ACH_GOAL_PRIMARY, a.get_goal(ACH_GOAL_PRIMARY));
        else
            a.set_progress(ACH_GOAL_PRIMARY, 0);
    }
    json_item *apples = json_find_item(group, "apples_eaten");
    if (apples)
    {
        int value = std::atoi(apples->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_EATEN);
        if (value > INT_MAX)
            value = INT_MAX;
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_free_groups(root);
    return (0);
}

