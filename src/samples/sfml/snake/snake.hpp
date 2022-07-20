#pragma once

#include <rpp/fwd.hpp>
#include <SFML/Graphics.hpp>

#include <rpp/schedulers/run_loop_scheduler.hpp>

#include <vector>
#include <variant>

struct Coordinates
{
    int x{};
    int y{};
};

static constexpr size_t s_rows_count    = 20;
static constexpr size_t s_columns_count = 30;

using Direction = Coordinates;

using SnakeBody = std::vector<Coordinates>;

inline rpp::schedulers::run_loop g_run_loop{};

struct PresentEvent
{
    bool   is_begin{};
    size_t frame_number{};
};

using CustomEvent = std::variant<PresentEvent, sf::Event>;

auto get_presents_stream(const auto& events)
{
    return events.filter([](const CustomEvent& ev) { return std::holds_alternative<PresentEvent>(ev); })
                 .map([](const CustomEvent&    ev) { return std::get<PresentEvent>(ev); });
}

rpp::dynamic_observable<sf::RectangleShape> get_shapes_to_draw(const rpp::dynamic_observable<CustomEvent>& events);