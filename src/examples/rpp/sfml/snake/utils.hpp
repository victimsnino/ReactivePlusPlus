#pragma once

#include <rpp/fwd.hpp>
#include <rpp/schedulers/run_loop.hpp>

#include <SFML/Graphics.hpp>

#include <variant>
#include <vector>

static constexpr size_t s_rows_count    = 20;
static constexpr size_t s_columns_count = 30;

struct coordinates
{
    int x{};
    int y{};

    bool operator==(const coordinates& other) const = default;
    bool operator!=(const coordinates& other) const = default;
};

using Direction = coordinates;
using SnakeBody = std::vector<coordinates>;

inline rpp::schedulers::run_loop g_run_loop{};

struct present_event
{
    size_t frame_number{};
};

using CustomEvent = std::variant<present_event, sf::Event>;

auto get_presents_stream(const auto& events)
{
    return events | rpp::ops::filter([](const CustomEvent& ev) { return std::holds_alternative<present_event>(ev); })
         | rpp::ops::map([](const CustomEvent& ev) { return std::get<present_event>(ev); });
}
