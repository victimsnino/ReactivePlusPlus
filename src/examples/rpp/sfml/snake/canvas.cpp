#include "canvas.hpp"

#include "snake.hpp"


static constexpr float s_gap_size  = 1.0f;
static constexpr float s_cell_size = 10.0f;

sf::RectangleShape get_rectangle_at(Coordinates location, sf::Color color)
{
    sf::RectangleShape box;
    box.setSize(sf::Vector2f{s_cell_size, s_cell_size});
    box.setPosition(sf::Vector2f{(s_cell_size + s_gap_size) * static_cast<float>(location.x), (s_cell_size + s_gap_size) * static_cast<float>(location.y)});
    box.setFillColor(color);
    return box;
}

sf::Vector2u get_window_size(size_t rows_count, size_t cols_count)
{
    return {static_cast<uint32_t>(static_cast<float>(cols_count) * (s_cell_size + s_gap_size)),
            static_cast<uint32_t>(static_cast<float>(rows_count) * (s_cell_size + s_gap_size))};
}