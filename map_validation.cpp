#include "map_validation.hpp"
#include "game_data.hpp"
#include <queue>
#include <utility>
#include <cstdlib>
#include <algorithm>

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

static int count_free_neighbors(
    const std::vector<std::string> &map, bool wrap_edges, int width, int height,
    int x, int y, const std::vector<std::vector<bool> > &visited) {
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    int count = 0;
    for (int dir = 0; dir < 4; ++dir) {
        int nx = x + dx[dir];
        int ny = y + dy[dir];
        if (wrap_edges) {
            if (nx < 0)
                nx = width - 1;
            else if (nx >= width)
                nx = 0;
            if (ny < 0)
                ny = height - 1;
            else if (ny >= height)
                ny = 0;
        } else {
            if (nx < 0 || ny < 0 || nx >= width || ny >= height)
                continue;
        }
        if (map[ny][nx] != MAP_TILE_WALL && !visited[ny][nx])
            count++;
    }
    return count;
}

static bool remaining_connected(
    const std::vector<std::string> &map, bool wrap_edges, int width, int height,
    const std::vector<std::vector<bool> > &visited, int tail_x, int tail_y,
    int remaining) {
    if (remaining == 0)
        return true;
    if (visited[tail_y][tail_x])
        return false;
    std::queue<std::pair<int, int> > q;
    std::vector<std::vector<bool> > seen(height,
                                         std::vector<bool>(width, false));
    q.emplace(tail_x, tail_y);
    seen[tail_y][tail_x] = true;
    int count = 0;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (; !q.empty(); q.pop()) {
        int x = q.front().first;
        int y = q.front().second;
        count++;
        for (int dir = 0; dir < 4; ++dir) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (wrap_edges) {
                if (nx < 0)
                    nx = width - 1;
                else if (nx >= width)
                    nx = 0;
                if (ny < 0)
                    ny = height - 1;
                else if (ny >= height)
                    ny = 0;
            } else {
                if (nx < 0 || ny < 0 || nx >= width || ny >= height)
                    continue;
            }
            if (seen[ny][nx] || visited[ny][nx] ||
                map[ny][nx] == MAP_TILE_WALL)
                continue;
            seen[ny][nx] = true;
            q.emplace(nx, ny);
        }
    }
    return count == remaining;
}

static bool dfs_return_path(const std::vector<std::string> &map, bool wrap_edges,
                            int x, int y, int tail_x, int tail_y,
                            int visited_count, int total,
                            std::vector<std::vector<bool> > &visited) {
    if (x == tail_x && y == tail_y)
        return visited_count == total;
    size_t width = map[0].size();
    size_t height = map.size();
    struct Move {
        int nx;
        int ny;
        std::vector<std::pair<int, int> > path;
        int neigh;
    };
    std::vector<Move> moves;
    const int dx[4] = {0, 1, 0, -1};
    const int dy[4] = {-1, 0, 1, 0};
    for (int dir = 0; dir < 4; ++dir) {
        int nx = x;
        int ny = y;
        std::vector<std::pair<int, int> > path;
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
                    ty >= static_cast<int>(height))
                    break;
            }
            char tile = map[ty][tx];
            if (tile == MAP_TILE_WALL || visited[ty][tx])
                break;
            path.emplace_back(tx, ty);
            nx = tx;
            ny = ty;
            if (nx == tail_x && ny == tail_y) {
                if (visited_count + static_cast<int>(path.size()) == total)
                    return true;
                path.clear();
                break;
            }
            if (tile != MAP_TILE_ICE)
                break;
        }
        if (path.empty())
            continue;
        for (const auto &p : path)
            visited[p.second][p.first] = true;
        int neigh = count_free_neighbors(map, wrap_edges, static_cast<int>(width),
                                         static_cast<int>(height), nx, ny,
                                         visited);
        for (const auto &p : path)
            visited[p.second][p.first] = false;
        if (neigh == 0)
            continue;
        moves.push_back({nx, ny, path, neigh});
    }
    std::sort(moves.begin(), moves.end(),
              [](const Move &a, const Move &b) { return a.neigh < b.neigh; });
    for (const auto &m : moves) {
        for (const auto &p : m.path)
            visited[p.second][p.first] = true;
        int new_count = visited_count + static_cast<int>(m.path.size());
        int remaining = total - new_count;
        bool ok = remaining_connected(map, wrap_edges, static_cast<int>(width),
                                      static_cast<int>(height), visited,
                                      tail_x, tail_y, remaining);
        if (ok &&
            dfs_return_path(map, wrap_edges, m.nx, m.ny, tail_x, tail_y,
                            new_count, total, visited))
            return true;
        for (const auto &p : m.path)
            visited[p.second][p.first] = false;
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
    for (size_t y = 0; y < height; ++y)
        for (size_t x = 0; x < width; ++x)
            if (map[y][x] != MAP_TILE_WALL)
                total++;
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
