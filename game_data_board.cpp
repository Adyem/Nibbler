#include "game_data.hpp"
#include "libft/RNG/RNG.hpp"
#include <algorithm>

void game_data::set_map_value(int x, int y, int layer, int value) {
    int prev_val = this->_map.get(x, y, layer);
    this->_map.set(x, y, layer, value);
    if (layer == 2) {
        if (prev_val == 0 && value != 0)
            this->remove_empty_cell(x, y);
        else if (prev_val != 0 && value == 0 &&
                 this->_map.get(x, y, 0) != GAME_TILE_WALL)
            this->add_empty_cell(x, y);
    } else if (layer == 0) {
        if (value == GAME_TILE_WALL)
            this->remove_empty_cell(x, y);
        else if (prev_val == GAME_TILE_WALL &&
                 this->_map.get(x, y, 2) == 0)
            this->add_empty_cell(x, y);
    }
    return;
}

int game_data::get_map_value(int x, int y, int layer) const {
    return (this->_map.get(x, y, layer));
}

void game_data::set_player_snake_length(int player, int length) {
    if (player < 0 || player >= 4)
        return;
    if (length < 0)
        length = 0;
    else if (length > MAX_SNAKE_LENGTH)
        length = MAX_SNAKE_LENGTH;
    this->_snake_length[player] = length;
}

void game_data::apply_snake_segments(int player, const std::vector<t_coordinates> &segments) {
    if (player < 0 || player >= 4)
        return;

    this->_snake_segments[player].clear();

    size_t limit = std::min<size_t>(segments.size(), static_cast<size_t>(MAX_SNAKE_LENGTH));
    for (size_t i = 0; i < limit; ++i)
        this->_snake_segments[player].push_back(segments[i]);

    this->_snake_length[player] = static_cast<int>(this->_snake_segments[player].size());

    this->write_snake_to_map(player);

    for (const t_coordinates &coord : this->_snake_segments[player])
        this->remove_empty_cell(coord.x, coord.y);
}

void game_data::rebuild_snake_segments_from_map(int player) {
    if (player < 0 || player >= 4)
        return;

    std::deque<t_coordinates> rebuilt;
    int offset = (player + 1) * 1000000;
    int max_value = offset + MAX_SNAKE_LENGTH;

    size_t width = this->_map.get_width();
    size_t height = this->_map.get_height();

    std::vector<std::pair<int, t_coordinates>> found;
    found.reserve(static_cast<size_t>(this->_snake_length[player]));

    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            int value = this->_map.get(x, y, 2);
            if (value >= offset + 1 && value <= max_value)
            {
                found.emplace_back(value, t_coordinates{static_cast<int>(x), static_cast<int>(y)});
            }
        }
    }

    std::sort(found.begin(), found.end(), [](const std::pair<int, t_coordinates> &lhs,
                                            const std::pair<int, t_coordinates> &rhs) {
        return lhs.first < rhs.first;
    });

    for (const auto &entry : found)
    {
        rebuilt.push_back(entry.second);
        if (rebuilt.size() >= static_cast<size_t>(MAX_SNAKE_LENGTH))
            break;
    }

    this->_snake_segments[player].swap(rebuilt);
    this->_snake_length[player] = static_cast<int>(this->_snake_segments[player].size());
    this->write_snake_to_map(player);
}

void game_data::sync_snake_segments_from_map() {
    for (int player = 0; player < 4; ++player)
        this->rebuild_snake_segments_from_map(player);
}

void game_data::write_snake_to_map(int player) {
    if (player < 0 || player >= 4)
        return;

    int offset = (player + 1) * 1000000;
    int index = 1;
    for (const t_coordinates &coord : this->_snake_segments[player])
    {
        this->_map.set(coord.x, coord.y, 2, offset + index);
        ++index;
    }
}

void game_data::reset_player_status_effects(int player) {
    if (player < 0 || player >= 4)
        return;
    this->_direction_moving[player] = DIRECTION_NONE;
    this->_direction_moving_ice[player] = 0;
    this->_speed_boost_steps[player] = 0;
    this->_fire_boost_active[player] = false;
    this->_frosty_steps[player] = 0;
    this->_update_timer[player] = 0.0;
}

void game_data::add_empty_cell(int x, int y) {
    size_t width = this->_map.get_width();
    int flat = y * static_cast<int>(width) + x;
    if (flat < 0 || flat >= static_cast<int>(this->_empty_cell_indices.size()))
        return;
    if (this->_empty_cell_indices[flat] != -1)
        return;
    if (this->_map.get(x, y, 2) != 0)
        return;
    if (this->_map.get(x, y, 0) != GAME_TILE_EMPTY)
        return;
    this->_empty_cells.push_back((t_coordinates){x, y});
    this->_empty_cell_indices[flat] = static_cast<int>(this->_empty_cells.size() - 1);
    return;
}

void game_data::remove_empty_cell(int x, int y) {
    size_t width = this->_map.get_width();
    int flat = y * static_cast<int>(width) + x;
    if (flat < 0 || flat >= static_cast<int>(this->_empty_cell_indices.size()))
        return;
    int idx = this->_empty_cell_indices[flat];
    if (idx == -1)
        return;
    int last = static_cast<int>(this->_empty_cells.size() - 1);
    if (idx != last) {
        this->_empty_cells[idx] = this->_empty_cells[last];
        int flat_last = this->_empty_cells[idx].y * static_cast<int>(width) +
                        this->_empty_cells[idx].x;
        this->_empty_cell_indices[flat_last] = idx;
    }
    this->_empty_cells.pop_back();
    this->_empty_cell_indices[flat] = -1;
    return;
}

void game_data::initialize_empty_cells() {
    size_t width = this->_map.get_width();
    size_t height = this->_map.get_height();
    this->_empty_cells.clear();
    this->_empty_cell_indices.assign(width * height, -1);
    size_t y = 0;
    while (y < height) {
        size_t x = 0;
        while (x < width) {
            if (this->_map.get(x, y, 2) == 0 &&
                this->_map.get(x, y, 0) == GAME_TILE_EMPTY) {
                this->_empty_cells.push_back((t_coordinates){static_cast<int>(x),
                                                             static_cast<int>(y)});
                this->_empty_cell_indices[y * width + x] =
                    static_cast<int>(this->_empty_cells.size() - 1);
            }
            ++x;
        }
        ++y;
    }
    return;
}

size_t game_data::get_width() const {
    return (this->_map.get_width());
}

size_t game_data::get_height() const {
    return (this->_map.get_height());
}

void game_data::reset_board() {
    size_t y = 0;
    while (y < this->_map.get_height()) {
        size_t x = 0;
        while (x < this->_map.get_width()) {
            this->_map.set(x, y, 0, GAME_TILE_EMPTY);
            this->_map.set(x, y, 1, 0);
            this->_map.set(x, y, 2, 0);
            ++x;
        }
        ++y;
    }
    int i = 0;
    while (i < 4) {
        this->reset_player_status_effects(i);
        this->_snake_segments[i].clear();
        this->_snake_length[i] = 0;
        ++i;
    }
    this->_amount_players_dead = 0;
    this->initialize_empty_cells();
    int mid_x = static_cast<int>(this->_map.get_width() / 2);
    int mid_y = static_cast<int>(this->_map.get_height() / 2);

    // Initialize snake with 4 segments in the middle
    std::vector<t_coordinates> initial_segments;
    initial_segments.push_back((t_coordinates){mid_x, mid_y});
    for (int j = 1; j < 4; j++) {
        int body_x = mid_x - j;
        if (body_x >= 0)
            initial_segments.push_back((t_coordinates){body_x, mid_y});
    }
    this->apply_snake_segments(0, initial_segments);

    this->spawn_food();
    if (this->_additional_food_items)
        this->spawn_fire_tile();
    return ;
}

void game_data::resize_board(int width, int height) {
    this->_map.resize(width, height, 3);
    if (this->_map.get_error()) {
        this->_error = this->_map.get_error();
        return;
    }
    this->reset_board();
    return;
}

bool game_data::is_tile_free(int &x, int &y) const {
    size_t width = this->_map.get_width();
    size_t height = this->_map.get_height();
    if (x < 0) {
        if (this->_wrap_around_edges)
            x = static_cast<int>(width) - 1;
        else
            return false;
    } else if (x >= static_cast<int>(width)) {
        if (this->_wrap_around_edges)
            x = 0;
        else
            return false;
    }
    if (y < 0) {
        if (this->_wrap_around_edges)
            y = static_cast<int>(height) - 1;
        else
            return false;
    } else if (y >= static_cast<int>(height)) {
        if (this->_wrap_around_edges)
            y = 0;
        else
            return false;
    }
    if (this->_map.get(x, y, 0) == GAME_TILE_WALL)
        return false;
    if (this->_map.get(x, y, 2) != 0)
        return false;
    return true;
}

bool game_data::can_spawn_frosty_food(int x, int y) {
    size_t width = this->_map.get_width();
    size_t height = this->_map.get_height();
    size_t wall_count = 0;
    for (size_t j = 0; j < height; ++j)
        for (size_t i = 0; i < width; ++i)
            if (this->_map.get(static_cast<int>(i), static_cast<int>(j), 0) == GAME_TILE_WALL)
                wall_count++;
    size_t open_tiles = width * height - wall_count;
    int snake_tiles = 0;
    for (int i = 0; i < 4; ++i)
        snake_tiles += this->_snake_length[i];
    if (snake_tiles >= static_cast<int>(open_tiles * 0.6))
        return false;
    const int dirs[4][2] = {{0,-1}, {1,0}, {0,1}, {-1,0}};
    for (int d = 0; d < 4; ++d) {
        int ax = x - dirs[d][0];
        int ay = y - dirs[d][1];
        if (!this->is_tile_free(ax, ay))
            continue;
        int tx = x + dirs[d][0];
        int ty = y + dirs[d][1];
        if (!this->is_tile_free(tx, ty))
            continue;
        int steps = 1;
        bool valid = true;
        while (steps < 2 || this->_map.get(tx, ty, 0) == GAME_TILE_ICE) {
            tx += dirs[d][0];
            ty += dirs[d][1];
            if (!this->is_tile_free(tx, ty)) {
                valid = false;
                break;
            }
            steps++;
            if (steps > static_cast<int>(open_tiles)) {
                valid = false;
                break;
            }
        }
        if (valid)
            return true;
    }
    return false;
}

void game_data::spawn_food() {
    while (!this->_empty_cells.empty())
    {
        int idx = ft_dice_roll(1, static_cast<int>(this->_empty_cells.size())) - 1;
        t_coordinates coord = this->_empty_cells[idx];
        if (this->_map.get(coord.x, coord.y, 2) != 0 ||
            this->_map.get(coord.x, coord.y, 0) != GAME_TILE_EMPTY)
        {
            this->remove_empty_cell(coord.x, coord.y);
            continue;
        }
        int item = FOOD;
        if (this->_additional_food_items)
        {
            int roll = ft_dice_roll(1, 3);
            if (roll == 2)
                item = FIRE_FOOD;
            else if (roll == 3)
            {
                bool can_frost = this->can_spawn_frosty_food(coord.x, coord.y);
                if (can_frost && this->_wrap_around_edges == 0)
                {
                    size_t width = this->_map.get_width();
                    size_t height = this->_map.get_height();
                    if ((coord.x == 0 || coord.x == static_cast<int>(width) - 1) &&
                        (coord.y == 0 || coord.y == static_cast<int>(height) - 1))
                        can_frost = false;
                }
                if (can_frost)
                    item = FROSTY_FOOD;
                else if (ft_dice_roll(1, 2) == 1)
                    item = FIRE_FOOD;
            }
        }
        this->_map.set(coord.x, coord.y, 2, item);
        this->remove_empty_cell(coord.x, coord.y);
        return;
    }
    return;
}

void game_data::spawn_fire_tile()
{
    if (this->_empty_cells.empty())
        return ;
    int idx = ft_dice_roll(1, static_cast<int>(this->_empty_cells.size())) - 1;
    t_coordinates coord = this->_empty_cells[idx];
    this->_map.set(coord.x, coord.y, 0, GAME_TILE_FIRE);
    this->remove_empty_cell(coord.x, coord.y);
    return;
}
