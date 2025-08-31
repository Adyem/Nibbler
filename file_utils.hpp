#pragma once

#include <vector>
#include <string>

int open_file_read(const char *path);
char **read_file_lines(const char *path);

struct game_rules {
    int error;
    int wrap_around_edges;
    int additional_fruits;
    int snake_length;
    std::vector<std::string> custom_map;
};

int read_game_rules(const char *path, game_rules &rules);
