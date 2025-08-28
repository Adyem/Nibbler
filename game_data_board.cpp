#include "game_data.hpp"
#include "libft/RNG/RNG.hpp"

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

void game_data::add_empty_cell(int x, int y) {
    size_t width = this->_map.get_width();
    int flat = y * static_cast<int>(width) + x;
    if (flat < 0 || flat >= static_cast<int>(this->_empty_cell_indices.size()))
        return;
    if (this->_empty_cell_indices[flat] != -1)
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
                this->_map.get(x, y, 0) != GAME_TILE_WALL) {
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
        this->_direction_moving[i] = DIRECTION_NONE;
        this->_direction_moving_ice[i] = 0;
        this->_speed_boost_steps[i] = 0;
        this->_frosty_steps[i] = 0;
        // Only initialize Player 1 snake, others are inactive (length 0)
        this->_snake_length[i] = (i == 0) ? 4 : 0;
        this->_update_timer[i] = 0.0;
        ++i;
    }
    this->_amount_players_dead = 0;
    this->initialize_empty_cells();
    int mid_x = static_cast<int>(this->_map.get_width() / 2);
    int mid_y = static_cast<int>(this->_map.get_height() / 2);

    // Initialize snake with 4 segments in the middle
    this->_map.set(mid_x, mid_y, 2, SNAKE_HEAD_PLAYER_1);
    this->remove_empty_cell(mid_x, mid_y);

    // Add 3 body segments to the left of the head
    // Snake segments are numbered: head=1000001, body=1000002, 1000003, 1000004, etc.
    for (int j = 1; j < 4; j++) {
        int body_x = mid_x - j;
        if (body_x >= 0) {
            this->_map.set(body_x, mid_y, 2, SNAKE_HEAD_PLAYER_1 + j);
            this->remove_empty_cell(body_x, mid_y);
        }
    }

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

void game_data::spawn_food() {
    if (this->_empty_cells.empty())
        return;
    int idx = ft_dice_roll(1, static_cast<int>(this->_empty_cells.size())) - 1;
    t_coordinates coord = this->_empty_cells[idx];
    int item = FOOD;
    if (this->_additional_food_items)
    {
        int roll = ft_dice_roll(1, 3);
        if (roll == 2)
            item = FIRE_FOOD;
        else if (roll == 3)
            item = FROSTY_FOOD;
        if (item == FROSTY_FOOD && this->_wrap_around_edges == 0)
        {
            size_t width = this->_map.get_width();
            size_t height = this->_map.get_height();
            if ((coord.x == 0 || coord.x == static_cast<int>(width) - 1) &&
                (coord.y == 0 || coord.y == static_cast<int>(height) - 1))
            {
                item = (ft_dice_roll(1, 2) == 1) ? FOOD : FIRE_FOOD;
            }
        }
    }
    this->_map.set(coord.x, coord.y, 2, item);
    this->remove_empty_cell(coord.x, coord.y);
    return ;
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
