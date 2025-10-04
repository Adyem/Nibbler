#include "game_data.hpp"
#include "libft/JSon/json.hpp"
#include <filesystem>
#include <cstdlib>
#include <string>
#include <cstring>
#include <limits>
#include <cerrno>
#include <charconv>
#include <string_view>
#include <cctype>

namespace {

bool parse_clamped_int(const char *valueStr, int minValue, int maxValue, int &out)
{
    if (!valueStr)
        return false;

    std::string_view view(valueStr);
    auto trim = [](std::string_view &sv) {
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front())))
            sv.remove_prefix(1);
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back())))
            sv.remove_suffix(1);
    };
    trim(view);
    if (view.empty())
        return false;

    long long parsed = 0;
    bool parsedSuccessfully = false;

    const char *begin = view.data();
    const char *end = begin + view.size();
    auto fcResult = std::from_chars(begin, end, parsed);
    if (fcResult.ec == std::errc() && fcResult.ptr == end)
    {
        parsedSuccessfully = true;
    }
    else if (fcResult.ec == std::errc::result_out_of_range)
    {
        parsed = (view.front() == '-') ? std::numeric_limits<long long>::min()
                                       : std::numeric_limits<long long>::max();
        parsedSuccessfully = true;
    }
    else
    {
        errno = 0;
        std::string buffer(view);
        char *endptr = nullptr;
        long long value = std::strtoll(buffer.c_str(), &endptr, 10);
        if (endptr != buffer.c_str() && endptr)
        {
            while (*endptr != '\0')
            {
                if (!std::isspace(static_cast<unsigned char>(*endptr)))
                    break;
                ++endptr;
            }
            if (*endptr == '\0')
            {
                parsed = value;
                parsedSuccessfully = true;
            }
        }
        if (errno == ERANGE && parsedSuccessfully)
        {
            parsed = (view.front() == '-') ? std::numeric_limits<long long>::min()
                                           : std::numeric_limits<long long>::max();
            parsedSuccessfully = true;
        }
    }

    if (!parsedSuccessfully)
        return false;

    if (parsed < static_cast<long long>(minValue))
        parsed = static_cast<long long>(minValue);
    else if (parsed > static_cast<long long>(maxValue))
        parsed = static_cast<long long>(maxValue);

    out = static_cast<int>(parsed);
    return true;
}

} // anonymous namespace

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
                this->_fire_boost_active[index] = false;
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
    int desiredSnakeLength = -1;
    json_item* len = json_find_item(group, "snake_length");
    if (len)
    {
        int parsed = 0;
        if (parse_clamped_int(len->value, 1, MAX_SNAKE_LENGTH, parsed))
            desiredSnakeLength = parsed;
    }
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
    auto parse_counter = [](json_item *item, int minValue, int maxValue, int defaultValue) {
        int parsed = 0;
        if (item && parse_clamped_int(item->value, minValue, maxValue, parsed))
            return parsed;
        return defaultValue;
    };

    const int maxCounter = std::numeric_limits<int>::max();

    json_item *apples = json_find_item(group, "apples_eaten");
    {
        int value = parse_counter(apples, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_EATEN);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *apples_normal = json_find_item(group, "apples_normal_eaten");
    {
        int value = parse_counter(apples_normal, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_NORMAL_EATEN);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *apples_frosty = json_find_item(group, "apples_frosty_eaten");
    {
        int value = parse_counter(apples_frosty, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_FROSTY_EATEN);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *apples_fire = json_find_item(group, "apples_fire_eaten");
    {
        int value = parse_counter(apples_fire, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_APPLES_FIRE_EATEN);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *steps_normal = json_find_item(group, "steps_normal");
    {
        int value = parse_counter(steps_normal, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_TILE_NORMAL_STEPS);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *steps_frosty = json_find_item(group, "steps_frosty");
    {
        int value = parse_counter(steps_frosty, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_TILE_FROSTY_STEPS);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    json_item *steps_fire = json_find_item(group, "steps_fire");
    {
        int value = parse_counter(steps_fire, 0, maxCounter, 0);
        ft_achievement &a =
            this->_character.get_achievements().at(ACH_TILE_FIRE_STEPS);
        a.set_progress(ACH_GOAL_PRIMARY, value);
    }
    this->reset_board();

    auto count_player_one_segments = [&]() -> int {
        int count = 0;
        size_t width = this->_map.get_width();
        size_t height = this->_map.get_height();
        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                int value = this->_map.get(x, y, 2);
                if (value >= SNAKE_HEAD_PLAYER_1 && value < SNAKE_HEAD_PLAYER_2)
                    ++count;
            }
        }
        return count;
    };

    auto find_segment_coordinates = [&](int segmentValue, t_coordinates &out) -> bool {
        size_t width = this->_map.get_width();
        size_t height = this->_map.get_height();
        for (size_t y = 0; y < height; ++y)
        {
            for (size_t x = 0; x < width; ++x)
            {
                if (this->_map.get(x, y, 2) == segmentValue)
                {
                    out.x = static_cast<int>(x);
                    out.y = static_cast<int>(y);
                    return true;
                }
            }
        }
        return false;
    };

    const int segmentOffset = SNAKE_HEAD_PLAYER_1 - 1;
    int currentSegmentCount = count_player_one_segments();

    if (desiredSnakeLength != -1)
    {
        if (desiredSnakeLength < currentSegmentCount)
        {
            while (currentSegmentCount > desiredSnakeLength)
            {
                t_coordinates tail;
                if (!find_segment_coordinates(segmentOffset + currentSegmentCount, tail))
                    break;
                this->set_map_value(tail.x, tail.y, 2, 0);
                --currentSegmentCount;
            }
        }
        else if (desiredSnakeLength > currentSegmentCount)
        {
            t_coordinates tail;
            if (find_segment_coordinates(segmentOffset + currentSegmentCount, tail))
            {
                int dx = -1;
                int dy = 0;
                if (currentSegmentCount > 1)
                {
                    t_coordinates beforeTail;
                    if (find_segment_coordinates(segmentOffset + currentSegmentCount - 1, beforeTail))
                    {
                        dx = tail.x - beforeTail.x;
                        dy = tail.y - beforeTail.y;
                    }
                }
                if (dx == 0 && dy == 0)
                {
                    dx = -1;
                    dy = 0;
                }
                int width = static_cast<int>(this->_map.get_width());
                int height = static_cast<int>(this->_map.get_height());
                while (currentSegmentCount < desiredSnakeLength)
                {
                    int nextX = tail.x + dx;
                    int nextY = tail.y + dy;
                    if (nextX < 0 || nextX >= width || nextY < 0 || nextY >= height)
                        break;
                    if (this->_map.get(nextX, nextY, 0) == GAME_TILE_WALL)
                        break;
                    if (this->_map.get(nextX, nextY, 2) != 0)
                        break;
                    ++currentSegmentCount;
                    tail.x = nextX;
                    tail.y = nextY;
                    this->set_map_value(nextX, nextY, 2, segmentOffset + currentSegmentCount);
                }
            }
        }
    }

    int actualCountPlaced = count_player_one_segments();
    this->set_player_snake_length(0, actualCountPlaced);
    this->sync_snake_segments_from_map();

    json_free_groups(root);
    return (0);
}
