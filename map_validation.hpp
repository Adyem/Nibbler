#pragma once

#include <vector>
#include <string>

bool validate_map_path(const std::vector<std::string> &map, bool wrap_edges);
bool validate_head_to_tail_path(const std::vector<std::string> &map,
                                bool wrap_edges, int head_x, int head_y,
                                int tail_x, int tail_y);
bool validate_snake_chain(int hx, int hy, int b1x, int b1y,
                          int b2x, int b2y, int b3x, int b3y,
                          size_t width, size_t height, bool wrap_edges);
