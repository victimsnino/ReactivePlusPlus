#include <SFML/Graphics.hpp>
#include <rpp/rpp.hpp>

#include "snake.hpp"
#include "canvas.hpp"

#include <iostream>




SnakeBody generate_initial_snake_body()
{
    // tail to head
    return SnakeBody{ Coordinates{1,1}, Coordinates{2,1}, Coordinates{3,1}, Coordinates{4,1} };
}

int wrap(int value, int max_value)
{
    return value < 0 ? max_value : value > max_value ? 0 : value;
}

SnakeBody&& move_snake(SnakeBody&& body, const Direction& direction)
{
    auto head = body.back();
    std::ranges::rotate(body, body.begin() + 1);

    head.x += direction.x;
    head.x = wrap(head.x, s_columns_count);

    head.y += direction.y;
    head.y = wrap(head.y, s_rows_count);

    body.back() = head;

    return std::move(body);
}

rpp::dynamic_observable<sf::RectangleShape> get_shapes_to_draw(const rpp::dynamic_observable<CustomEvent>& events)
{
    const auto key_event = events.filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
                                 .map([](const CustomEvent& ev) { return std::get<sf::Event>(ev); })
                                 .filter([](const sf::Event& event) { return event.type == sf::Event::KeyPressed; })
                                 .map([](const sf::Event& event) { return event.key; });

    const std::map<sf::Keyboard::Key, Direction> key_to_direction
    {
        {sf::Keyboard::Key::Right, { 1,  0}},
        {sf::Keyboard::Key::Left,  {-1,  0}},
        {sf::Keyboard::Key::Down,  { 0,  1}},
        {sf::Keyboard::Key::Up,    { 0, -1}},
    };

    const auto direction = key_event.filter([](const sf::Event::KeyEvent& key_event)
                                    {
                                        return !key_event.alt && !key_event.control && !key_event.shift && !key_event.system;
                                    })
                                    .map([key_to_direction](const sf::Event::KeyEvent& event)-> std::optional<Direction>
                                    {
                                        const auto itr = key_to_direction.find(event.code);
                                        if (itr != key_to_direction.cend())
                                            return itr->second;
                                        return std::nullopt;
                                    })
                                    .filter([](const auto& optional) { return optional.has_value();  })
                                    .map([](const auto& optional) { return optional.value(); })
                                    .start_with(key_to_direction.at(sf::Keyboard::Key::Right))
                                    /*.distinct_until_changed()*/;

    auto snake = rpp::source::interval(std::chrono::milliseconds{200}, g_run_loop)
                 .with_latest_from([](const auto&, const Direction& direction) { return direction; }, direction)
                 .scan(generate_initial_snake_body(), &move_snake);

    return get_presents_stream(events)
           .filter([](const PresentEvent&  ev) { return ev.is_begin; })
           .with_latest_from([](const auto&, const SnakeBody& snake_body) { return snake_body; }, std::move(snake))
           .flat_map([](const SnakeBody&   body) { return rpp::source::from_iterable(body); })
           .map([](const Coordinates&      coords) { return get_rectangle_at(coords, sf::Color::Red); });
}
