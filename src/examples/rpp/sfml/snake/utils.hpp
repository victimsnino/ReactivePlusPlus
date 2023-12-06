#pragma once

#include <rpp/fwd.hpp>
#include <SFML/Graphics.hpp>

#include <rpp/schedulers/run_loop.hpp>

#include <vector>
#include <variant>

static constexpr size_t s_rows_count = 20;
static constexpr size_t s_columns_count = 30;

struct Coordinates
{
    int x{};
    int y{};

    bool operator==(const Coordinates& other) const = default;
    bool operator!=(const Coordinates& other) const = default;
};

using Direction = Coordinates;
using SnakeBody = std::vector<Coordinates>;

inline rpp::schedulers::run_loop g_run_loop{};

struct PresentEvent
{
    size_t frame_number{};
};

using CustomEvent = std::variant<PresentEvent, sf::Event>;

auto get_presents_stream(const auto& events)
{
    return events | rpp::ops::filter([](const CustomEvent& ev) { return std::holds_alternative<PresentEvent>(ev); })
                  | rpp::ops::map([](const CustomEvent&    ev) { return std::get<PresentEvent>(ev); });
}