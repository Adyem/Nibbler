#include "file_utils.hpp"
#include "game_data.hpp"
#include "map_validation.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

int open_file_read(const char *path) {
    if (!path) {
        return -1;
    }
    return open(path, O_RDONLY);
}

char **read_file_lines(const char *path) {
    int fd = open_file_read(path);
    if (fd < 0) {
        return nullptr;
    }
    FILE *fp = fdopen(fd, "r");
    if (!fp) {
        close(fd);
        return nullptr;
    }
    std::vector<std::string> lines;
    char *buffer = nullptr;
    size_t buf_size = 0;
    ssize_t line_len;
    while ((line_len = getline(&buffer, &buf_size, fp)) != -1) {
        if (line_len > 0 && buffer[line_len - 1] == '\n') {
            buffer[line_len - 1] = '\0';
        }
        lines.emplace_back(buffer);
    }
    free(buffer);
    fclose(fp);

    char **result = new char *[lines.size() + 1];
    for (size_t i = 0; i < lines.size(); ++i) {
        result[i] = new char[lines[i].size() + 1];
        std::copy(lines[i].begin(), lines[i].end(), result[i]);
        result[i][lines[i].size()] = '\0';
    }
    result[lines.size()] = nullptr;
    return result;
}

int read_game_rules(const char *path, game_rules &rules) {
    rules.error = 0;
    rules.snake_length = 4;
    int fd = open_file_read(path);
    if (fd < 0) {
        rules.error = 1;
        return -1;
    }
    FILE *fp = fdopen(fd, "r");
    if (!fp) {
        close(fd);
        rules.error = 1;
        return -1;
    }
    char *line = nullptr;
    size_t buf_size = 0;
    ssize_t line_len;
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
    while ((line_len = getline(&line, &buf_size, fp)) != -1) {
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
        }
        if (!in_map) {
            if (std::strncmp(line, "WRAP_AROUND_EDGES=", 19) == 0) {
                rules.wrap_around_edges = (line[19] == '1');
                found++;
            } else if (std::strncmp(line, "ADDITIONAL_FRUITS=", 20) == 0) {
                rules.additional_fruits = (line[20] == '1');
                found++;
            } else if (std::strncmp(line, "CUSTOM_MAP=", 11) == 0) {
                in_map = true;
            }
        } else {
            size_t len = std::strlen(line);
            if (len == 0) {
                free(line);
                fclose(fp);
                rules.error = 1;
                return -1;
            }
            if (map_width == 0)
                map_width = len;
            else if (len != map_width) {
                free(line);
                fclose(fp);
                rules.error = 1;
                return -1;
            }
            size_t y = rules.custom_map.size();
            for (size_t i = 0; i < len; ++i) {
                char c = line[i];
                if (c != MAP_TILE_EMPTY && c != MAP_TILE_WALL &&
                    c != MAP_TILE_ICE && c != MAP_TILE_FIRE &&
                    c != MAP_TILE_SNAKE_HEAD &&
                    c != MAP_TILE_SNAKE_BODY_1 &&
                    c != MAP_TILE_SNAKE_BODY_2 &&
                    c != MAP_TILE_SNAKE_BODY_3) {
                    free(line);
                    fclose(fp);
                    rules.error = 1;
                    return -1;
                }
                if (c == MAP_TILE_SNAKE_HEAD) {
                    head_count++;
                    if (head_count > 1) {
                        free(line);
                        fclose(fp);
                        rules.error = 1;
                        return -1;
                    }
                    head_x = static_cast<int>(i);
                    head_y = static_cast<int>(y);
                } else if (c == MAP_TILE_SNAKE_BODY_1) {
                    body1_count++;
                    if (body1_count > 1) {
                        free(line);
                        fclose(fp);
                        rules.error = 1;
                        return -1;
                    }
                    body1_x = static_cast<int>(i);
                    body1_y = static_cast<int>(y);
                } else if (c == MAP_TILE_SNAKE_BODY_2) {
                    body2_count++;
                    if (body2_count > 1) {
                        free(line);
                        fclose(fp);
                        rules.error = 1;
                        return -1;
                    }
                    body2_x = static_cast<int>(i);
                    body2_y = static_cast<int>(y);
                } else if (c == MAP_TILE_SNAKE_BODY_3) {
                    body3_count++;
                    if (body3_count > 1) {
                        free(line);
                        fclose(fp);
                        rules.error = 1;
                        return -1;
                    }
                    body3_x = static_cast<int>(i);
                    body3_y = static_cast<int>(y);
                }
            }
            rules.custom_map.push_back(std::string(line));
        }
    }
    free(line);
    fclose(fp);
    if (in_map) {
        if (map_width < 5 || rules.custom_map.size() < 5 || head_count != 1 ||
            body1_count != 1 || body2_count != 1 || body3_count != 1 ||
            head_count + body1_count + body2_count + body3_count !=
                rules.snake_length ||
            !validate_snake_chain(head_x, head_y, body1_x, body1_y, body2_x,
                                  body2_y, body3_x, body3_y, map_width,
                                  rules.custom_map.size(),
                                  rules.wrap_around_edges) ||
            !validate_map_path(rules.custom_map, rules.wrap_around_edges) ||
            !validate_head_to_tail_path(rules.custom_map,
                                        rules.wrap_around_edges, head_x,
                                        head_y, body3_x, body3_y)) {
            rules.error = 1;
            return -1;
        }
    }
    if (found < 2) {
        rules.error = 1;
        return -1;
    }
    return 0;
}
