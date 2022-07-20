#pragma once

#include <vector>

struct Coordinates
{
    int x{};
    int y{};
};

static constexpr size_t s_rows_count = 20;
static constexpr size_t s_columns_count = 30;

using Direction = Coordinates;

using SnakeBody = std::vector<Coordinates>;