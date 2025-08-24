#include "game_data.hpp"
#include "libft/JSon/json.hpp"
#include <filesystem>
#include <cstdlib>
#include <string>
#include <cstring>
#include <limits>

static std::filesystem::path get_save_dir() {
    return (std::filesystem::current_path() / "save_data");
}

static void ensure_save_dir_exists() {
    std::filesystem::path dir = get_save_dir();
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    return;
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
        apple.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
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
        ft_achievement apple_normal;
        apple_normal.set_id(ACH_APPLES_NORMAL_EATEN);
        apple_normal.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
        ach.insert(ACH_APPLES_NORMAL_EATEN, apple_normal);
        if (ach.get_error())
                this->_error = ach.get_error();
        ft_achievement apple_frosty;
        apple_frosty.set_id(ACH_APPLES_FROSTY_EATEN);
        apple_frosty.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
        ach.insert(ACH_APPLES_FROSTY_EATEN, apple_frosty);
        if (ach.get_error())
                this->_error = ach.get_error();
        ft_achievement apple_fire;
        apple_fire.set_id(ACH_APPLES_FIRE_EATEN);
        apple_fire.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
        ach.insert(ACH_APPLES_FIRE_EATEN, apple_fire);
        if (ach.get_error())
                this->_error = ach.get_error();
        ft_achievement tile_normal;
        tile_normal.set_id(ACH_TILE_NORMAL_STEPS);
        tile_normal.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
        ach.insert(ACH_TILE_NORMAL_STEPS, tile_normal);
        if (ach.get_error())
                this->_error = ach.get_error();
        ft_achievement tile_frosty;
        tile_frosty.set_id(ACH_TILE_FROSTY_STEPS);
        tile_frosty.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
        ach.insert(ACH_TILE_FROSTY_STEPS, tile_frosty);
        if (ach.get_error())
                this->_error = ach.get_error();
        ft_achievement tile_fire;
        tile_fire.set_id(ACH_TILE_FIRE_STEPS);
        tile_fire.set_goal(ACH_GOAL_PRIMARY, std::numeric_limits<int>::max());
        ach.insert(ACH_TILE_FIRE_STEPS, tile_fire);
        if (ach.get_error())
                this->_error = ach.get_error();
        ensure_save_dir_exists();
        this->reset_board();
        return ;
}

int game_data::save_game() const {
    ensure_save_dir_exists();
    std::filesystem::path file = get_save_dir() /
                                 (std::string(this->_profile_name.c_str()) + ".json");
    json_group* group = json_create_json_group("game");
    if (!group)
        return (1);
    json_add_item_to_group(group, json_create_item
                        ("snake_length", this->_snake_length[0]));
    const ft_map<int, ft_achievement> &achievements =
        this->_character.get_achievements();
    const Pair<int, ft_achievement> *apple =
        achievements.find(ACH_APPLES_EATEN);
    int appleCount = apple ? apple->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *appleNormal =
        achievements.find(ACH_APPLES_NORMAL_EATEN);
    int appleNormalCount = appleNormal ? appleNormal->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *appleFrosty =
        achievements.find(ACH_APPLES_FROSTY_EATEN);
    int appleFrostyCount = appleFrosty ? appleFrosty->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *appleFire =
        achievements.find(ACH_APPLES_FIRE_EATEN);
    int appleFireCount = appleFire ? appleFire->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *tileNormal =
        achievements.find(ACH_TILE_NORMAL_STEPS);
    int tileNormalCount = tileNormal ? tileNormal->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *tileFrosty =
        achievements.find(ACH_TILE_FROSTY_STEPS);
    int tileFrostyCount = tileFrosty ? tileFrosty->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *tileFire =
        achievements.find(ACH_TILE_FIRE_STEPS);
    int tileFireCount = tileFire ? tileFire->value.get_progress(ACH_GOAL_PRIMARY) : 0;
    const Pair<int, ft_achievement> *snake =
        achievements.find(ACH_SNAKE_50);
    bool snake50 = snake ?
        snake->value.is_goal_complete(ACH_GOAL_PRIMARY) : false;
    json_add_item_to_group(group, json_create_item
                        ("achievement_snake50", snake50));
    json_add_item_to_group(group, json_create_item
                        ("apples_eaten", appleCount));
    json_add_item_to_group(group, json_create_item
                        ("apples_normal_eaten", appleNormalCount));
    json_add_item_to_group(group, json_create_item
                        ("apples_frosty_eaten", appleFrostyCount));
    json_add_item_to_group(group, json_create_item
                        ("apples_fire_eaten", appleFireCount));
    json_add_item_to_group(group, json_create_item
                        ("steps_normal", tileNormalCount));
    json_add_item_to_group(group, json_create_item
                        ("steps_frosty", tileFrostyCount));
    json_add_item_to_group(group, json_create_item
                        ("steps_fire", tileFireCount));
    int ret = json_write_to_file(file.c_str(), group);
    json_free_groups(group);
    return (ret);
}

int game_data::load_game() {
    ensure_save_dir_exists();
    std::filesystem::path file = get_save_dir() /
                                 (std::string(this->_profile_name.c_str()) + ".json");
    json_group* root = json_read_from_file(file.c_str());
    if (!root)
        return (1);
    json_group* group = json_find_group(root, "game");
    if (!group) {
        json_free_groups(root);
        return (1);
    }
    json_item* len = json_find_item(group, "snake_length");
    if (len)
        this->_snake_length[0] = std::atoi(len->value);
    json_item* ach = json_find_item(group, "achievement_snake50");
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
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *apples_normal = json_find_item(group, "apples_normal_eaten");
    if (apples_normal)
    {
        int value = std::atoi(apples_normal->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_NORMAL_EATEN);
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *apples_frosty = json_find_item(group, "apples_frosty_eaten");
    if (apples_frosty)
    {
        int value = std::atoi(apples_frosty->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_FROSTY_EATEN);
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *apples_fire = json_find_item(group, "apples_fire_eaten");
    if (apples_fire)
    {
        int value = std::atoi(apples_fire->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_FIRE_EATEN);
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *steps_normal = json_find_item(group, "steps_normal");
    if (steps_normal)
    {
        int value = std::atoi(steps_normal->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_TILE_NORMAL_STEPS);
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *steps_frosty = json_find_item(group, "steps_frosty");
    if (steps_frosty)
    {
        int value = std::atoi(steps_frosty->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_TILE_FROSTY_STEPS);
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *steps_fire = json_find_item(group, "steps_fire");
    if (steps_fire)
    {
        int value = std::atoi(steps_fire->value);
        if (value < 0)
            value = 0;
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_TILE_FIRE_STEPS);
        if (value > std::numeric_limits<int>::max())
            value = std::numeric_limits<int>::max();
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_free_groups(root);
    return (0);
}
