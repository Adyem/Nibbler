#include "file_utils.hpp"
#include "game_data.hpp"
#include "map_validation.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <cctype>
#include "libft/RNG/RNG.hpp"

std::optional<std::vector<std::string>> read_file_lines(const char *path) {
    if (!path)
        return std::nullopt;
    std::ifstream file(path);
    if (!file.is_open())
        return std::nullopt;
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

static std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
        ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
        --end;
    return s.substr(start, end - start);
}

static int parse_game_rules_lines(const std::vector<std::string> &lines,
                                  game_rules &rules) {
    rules.error = 0;
    rules.snake_length = 4;
    rules.error_message.clear();
    int found = 0;
    bool in_map = false;
    size_t map_width = 0;
    int head_count = 0;
    int body1_count = 0;
    int body2_count = 0;
    int body3_count = 0;
    int head_x = -1, head_y = -1;
    int body1_x = -1, body1_y = -1;
    int body2_x = -1, body2_y = -1;
    int body3_x = -1, body3_y = -1;
    rules.custom_map.clear();

    std::unordered_map<std::string, int> key_table = {
        {"WRAP_AROUND_EDGES", 0}, {"ADDITIONAL_FRUITS", 1}, {"CUSTOM_MAP", 2}};

    auto fail = [&](const std::string &msg) {
        rules.error = 1;
        rules.error_message = msg;
        return -1;
    };

    for (size_t idx = 0; idx < lines.size(); ++idx) {
        const std::string &raw_line = lines[idx];
        std::string trimmed = trim(raw_line);
        if (!in_map) {
            if (trimmed.empty() || trimmed[0] == '#')
                continue;
            size_t pos = trimmed.find('=');
            if (pos == std::string::npos)
                return fail("Invalid rule line: missing '=' delimiter");
            std::string key = trim(trimmed.substr(0, pos));
            std::string value = trim(trimmed.substr(pos + 1));
            auto it = key_table.find(key);
            if (it == key_table.end())
                return fail(std::string("Unknown rule key: ") + key);
            switch (it->second) {
            case 0:
                if (value != "0" && value != "1")
                    return fail("WRAP_AROUND_EDGES must be 0 or 1");
                rules.wrap_around_edges = (value == "1");
                found++;
                break;
            case 1:
                if (value != "0" && value != "1")
                    return fail("ADDITIONAL_FRUITS must be 0 or 1");
                rules.additional_fruits = (value == "1");
                found++;
                break;
            case 2:
                in_map = true;
                break;
            }
        } else {
            const std::string &line = raw_line;
            size_t len = line.size();
            if (len == 0)
                return fail("Custom map contains an empty line");
            if (map_width == 0)
                map_width = len;
            else if (len != map_width)
                return fail("Custom map rows must have consistent width");
            size_t y = rules.custom_map.size();
            for (size_t i = 0; i < len; ++i) {
                char c = line[i];
                if (c != MAP_TILE_EMPTY && c != MAP_TILE_WALL &&
                    c != MAP_TILE_ICE && c != MAP_TILE_FIRE &&
                    c != MAP_TILE_SNAKE_HEAD &&
                    c != MAP_TILE_SNAKE_BODY_1 &&
                    c != MAP_TILE_SNAKE_BODY_2 &&
                    c != MAP_TILE_SNAKE_BODY_3) {
                    return fail("Custom map contains invalid tile character");
                }
                if (c == MAP_TILE_SNAKE_HEAD) {
                    head_count++;
                    if (head_count > 1)
                        return fail("Custom map must contain exactly one snake head");
                    head_x = static_cast<int>(i);
                    head_y = static_cast<int>(y);
                } else if (c == MAP_TILE_SNAKE_BODY_1) {
                    body1_count++;
                    if (body1_count > 1)
                        return fail("Custom map must contain exactly one body segment 1");
                    body1_x = static_cast<int>(i);
                    body1_y = static_cast<int>(y);
                } else if (c == MAP_TILE_SNAKE_BODY_2) {
                    body2_count++;
                    if (body2_count > 1)
                        return fail("Custom map must contain exactly one body segment 2");
                    body2_x = static_cast<int>(i);
                    body2_y = static_cast<int>(y);
                } else if (c == MAP_TILE_SNAKE_BODY_3) {
                    body3_count++;
                    if (body3_count > 1)
                        return fail("Custom map must contain exactly one body segment 3");
                    body3_x = static_cast<int>(i);
                    body3_y = static_cast<int>(y);
                }
            }
            // store the map line
            rules.custom_map.push_back(line);
        }
    }
    if (in_map) {
        const size_t MIN_DIM = 10;
        const size_t MAX_DIM = 30;
        size_t map_height = rules.custom_map.size();
        if (map_width < MIN_DIM || map_width > MAX_DIM ||
            map_height < MIN_DIM || map_height > MAX_DIM ||
            head_count != 1 ||
            body1_count != 1 || body2_count != 1 || body3_count != 1 ||
            head_count + body1_count + body2_count + body3_count !=
                rules.snake_length ||
            !validate_snake_chain(head_x, head_y, body1_x, body1_y, body2_x,
                                  body2_y, body3_x, body3_y, map_width,
                                  map_height,
                                  rules.wrap_around_edges) ||
            !validate_map_path(rules.custom_map, rules.wrap_around_edges) ||
            !validate_head_to_tail_path(rules.custom_map,
                                        rules.wrap_around_edges, head_x,
                                        head_y, body3_x, body3_y)) {
            return fail("Custom map validation failed (size/structure requirements not met)");
        }
    }
    if (found < 2) {
        return fail("Missing required rules (expected WRAP_AROUND_EDGES and ADDITIONAL_FRUITS)");
    }
    return 0;
}

int read_game_rules(game_data &data, game_rules &rules) {
    auto lines = read_file_lines(data.get_map_name());
    if (!lines) {
        rules.error = 1;
        rules.error_message = "Unable to read bonus map file";
        return -1;
    }
    return parse_game_rules_lines(*lines, rules);
}

int load_rules_into_game_data(game_data &data) {
    game_rules rules;
    if (read_game_rules(data, rules) < 0)
        return -1;
    return load_rules_into_game_data(data, rules);
}

int load_rules_into_game_data(game_data &data, const game_rules &rules) {
    if (!rules.custom_map.empty()) {
        size_t width = rules.custom_map[0].size();
        size_t height = rules.custom_map.size();
        data.resize_board(static_cast<int>(width),
                          static_cast<int>(height));
    }
    data.set_wrap_around_edges(rules.wrap_around_edges);
    data.set_additional_food_items(rules.additional_fruits);
    if (!rules.custom_map.empty()) {
        size_t width = rules.custom_map[0].size();
        size_t height = rules.custom_map.size();
        int player1Segments = 0;
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                char c = rules.custom_map[y][x];
                int tile = GAME_TILE_EMPTY;
                if (c == MAP_TILE_WALL)
                    tile = GAME_TILE_WALL;
                else if (c == MAP_TILE_ICE)
                    tile = GAME_TILE_ICE;
                else if (c == MAP_TILE_FIRE)
                    tile = GAME_TILE_FIRE;
                data.set_map_value(static_cast<int>(x), static_cast<int>(y), 0,
                                    tile);
                int snake = 0;
                if (c == MAP_TILE_SNAKE_HEAD)
                    snake = SNAKE_HEAD_PLAYER_1;
                else if (c == MAP_TILE_SNAKE_BODY_1)
                    snake = SNAKE_HEAD_PLAYER_1 + 1;
                else if (c == MAP_TILE_SNAKE_BODY_2)
                    snake = SNAKE_HEAD_PLAYER_1 + 2;
                else if (c == MAP_TILE_SNAKE_BODY_3)
                    snake = SNAKE_HEAD_PLAYER_1 + 3;
                data.set_map_value(static_cast<int>(x), static_cast<int>(y), 2,
                                    snake);
                if (snake >= SNAKE_HEAD_PLAYER_1 && snake < SNAKE_HEAD_PLAYER_2)
                    ++player1Segments;
            }
        }

        data.set_player_snake_length(0, player1Segments);
        for (int player = 1; player < 4; ++player)
            data.set_player_snake_length(player, 0);

        // After applying the map, ensure at least one food spawns at a random empty cell
        std::vector<std::pair<int,int>> empties;
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                if (data.get_map_value(static_cast<int>(x), static_cast<int>(y), 2) == 0 &&
                    data.get_map_value(static_cast<int>(x), static_cast<int>(y), 0) == GAME_TILE_EMPTY) {
                    empties.emplace_back(static_cast<int>(x), static_cast<int>(y));
                }
            }
        }
        if (!empties.empty()) {
            int idx = ft_dice_roll(1, static_cast<int>(empties.size())) - 1;
            int fx = empties[idx].first;
            int fy = empties[idx].second;
            data.set_map_value(fx, fy, 2, FOOD);
        }

        // If additional fruits are enabled, also spawn one fire tile randomly
        if (rules.additional_fruits) {
            std::vector<std::pair<int,int>> empties2;
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    if (data.get_map_value(static_cast<int>(x), static_cast<int>(y), 2) == 0 &&
                        data.get_map_value(static_cast<int>(x), static_cast<int>(y), 0) == GAME_TILE_EMPTY) {
                        empties2.emplace_back(static_cast<int>(x), static_cast<int>(y));
                    }
                }
            }
            if (!empties2.empty()) {
                int idx2 = ft_dice_roll(1, static_cast<int>(empties2.size())) - 1;
                int fx2 = empties2[idx2].first;
                int fy2 = empties2[idx2].second;
                data.set_map_value(fx2, fy2, 0, GAME_TILE_FIRE);
            }
        }
    }
    return 0;
}
