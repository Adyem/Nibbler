#include "map_validation.hpp"
#include "game_data.hpp"
#include <queue>
#include <utility>
#include <cstdlib>

static bool locate_head_and_total(const std::vector<std::string> &map,
                                  int &head_x, int &head_y, int &total) {
    size_t height = map.size();
    if (height == 0)
        return false;
    size_t width = map[0].size();
    head_x = -1;
    head_y = -1;
    total = 0;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            char c = map[y][x];
            if (c != MAP_TILE_WALL)
                total++;
            if (c == MAP_TILE_SNAKE_HEAD) {
                head_x = static_cast<int>(x);
                head_y = static_cast<int>(y);
            }
        }
    }
    return head_x >= 0 && head_y >= 0;
}

static void process_direction(
    const std::vector<std::string> &map, bool wrap_edges, int width, int height,
    int x, int y, int dir, std::vector<std::vector<bool> > &visited,
    std::vector<std::vector<bool> > &queued,
    std::queue<std::pair<int, int> > &q, int &visited_count) {
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    int nx = x;
    int ny = y;
    while (true) {
        int tx = nx + dx[dir];
        int ty = ny + dy[dir];
        if (wrap_edges) {
            if (tx < 0)
                tx = width - 1;
            else if (tx >= width)
                tx = 0;
            if (ty < 0)
                ty = height - 1;
            else if (ty >= height)
                ty = 0;
        } else {
            if (tx < 0 || ty < 0 || tx >= width || ty >= height)
                break;
        }
        char tile = map[ty][tx];
        if (tile == MAP_TILE_WALL || visited[ty][tx])
            break;
        visited[ty][tx] = true;
        visited_count++;
        nx = tx;
        ny = ty;
        if (tile != MAP_TILE_ICE) {
            if (!queued[ny][nx]) {
                q.emplace(nx, ny);
                queued[ny][nx] = true;
            }
            break;
        }
    }
}

static bool bfs_visit_all(const std::vector<std::string> &map, bool wrap_edges,
                          int head_x, int head_y, int total) {
    size_t height = map.size();
    size_t width = map[0].size();
    std::vector<std::vector<bool> > visited(height,
                                            std::vector<bool>(width, false));
    std::vector<std::vector<bool> > queued(height,
                                           std::vector<bool>(width, false));
    std::queue<std::pair<int, int> > q;
    q.emplace(head_x, head_y);
    queued[head_y][head_x] = true;
    visited[head_y][head_x] = true;
    int visited_count = 1;
    for (; !q.empty(); q.pop()) {
        int x = q.front().first;
        int y = q.front().second;
        for (int dir = 0; dir < 4; ++dir)
            process_direction(map, wrap_edges, static_cast<int>(width),
                              static_cast<int>(height), x, y, dir, visited,
                              queued, q, visited_count);
    }
    return visited_count == total;
}

bool validate_map_path(const std::vector<std::string> &map, bool wrap_edges) {
    int head_x, head_y, total;
    if (!locate_head_and_total(map, head_x, head_y, total))
        return false;
    return bfs_visit_all(map, wrap_edges, head_x, head_y, total);
}

static bool dfs_return_path(const std::vector<std::string> &map, bool wrap_edges,
                            int x, int y, int tail_x, int tail_y,
                            int visited_count, int total,
                            std::vector<std::vector<bool> > &visited) {
    if (x == tail_x && y == tail_y)
        return visited_count == total;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    size_t width = map[0].size();
    size_t height = map.size();
    for (int dir = 0; dir < 4; ++dir) {
        int nx = x;
        int ny = y;
        std::vector<std::pair<int, int> > path;
        bool invalid = false;
        while (true) {
            int tx = nx + dx[dir];
            int ty = ny + dy[dir];
            if (wrap_edges) {
                if (tx < 0)
                    tx = static_cast<int>(width) - 1;
                else if (tx >= static_cast<int>(width))
                    tx = 0;
                if (ty < 0)
                    ty = static_cast<int>(height) - 1;
                else if (ty >= static_cast<int>(height))
                    ty = 0;
            } else {
                if (tx < 0 || ty < 0 || tx >= static_cast<int>(width) ||
                    ty >= static_cast<int>(height)) {
                    invalid = true;
                    break;
                }
            }
            char tile = map[ty][tx];
            if (tile == MAP_TILE_WALL || visited[ty][tx]) {
                invalid = true;
                break;
            }
            path.emplace_back(tx, ty);
            nx = tx;
            ny = ty;
            if (nx == tail_x && ny == tail_y) {
                if (visited_count + static_cast<int>(path.size()) == total)
                    return true;
                invalid = true;
                break;
            }
            if (tile != MAP_TILE_ICE)
                break;
        }
        if (invalid || path.empty())
            continue;
        for (size_t i = 0; i < path.size(); ++i)
            visited[path[i].second][path[i].first] = true;
        if (dfs_return_path(map, wrap_edges, nx, ny, tail_x, tail_y,
                            visited_count + static_cast<int>(path.size()),
                            total, visited))
            return true;
        for (size_t i = 0; i < path.size(); ++i)
            visited[path[i].second][path[i].first] = false;
    }
    return false;
}

bool validate_head_to_tail_path(const std::vector<std::string> &map,
                                bool wrap_edges, int head_x, int head_y,
                                int tail_x, int tail_y) {
    size_t height = map.size();
    if (height == 0)
        return false;
    size_t width = map[0].size();
    int total = 0;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (map[y][x] != MAP_TILE_WALL)
                total++;
        }
    }
    std::vector<std::vector<bool> > visited(height,
                                            std::vector<bool>(width, false));
    visited[head_y][head_x] = true;
    return dfs_return_path(map, wrap_edges, head_x, head_y, tail_x, tail_y, 1,
                           total, visited);
}

bool validate_snake_chain(int hx, int hy, int b1x, int b1y, int b2x, int b2y,
                          int b3x, int b3y, size_t width, size_t height,
                          bool wrap_edges) {
    auto adjacent = [&](int ax, int ay, int bx, int by) {
        int dx = std::abs(ax - bx);
        int dy = std::abs(ay - by);
        if (wrap_edges) {
            if (dx == static_cast<int>(width) - 1)
                dx = 1;
            if (dy == static_cast<int>(height) - 1)
                dy = 1;
        }
        return dx + dy == 1;
    };
    return adjacent(hx, hy, b1x, b1y) &&
           adjacent(b1x, b1y, b2x, b2y) &&
           adjacent(b2x, b2y, b3x, b3y);
}
