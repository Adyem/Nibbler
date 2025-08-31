#pragma once

#include <vector>
#include <string>

class game_data;

int open_file_read(const char *path);
char **read_file_lines(const char *path);

struct game_rules {
    int error;
    int wrap_around_edges;
    int additional_fruits;
    int snake_length;
    std::vector<std::string> custom_map;
};

int read_game_rules(game_data &data, game_rules &rules);
int load_rules_into_game_data(game_data &data);
