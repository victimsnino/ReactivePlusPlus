#include "snake.hpp"
#include "canvas.hpp"
#include "utils.hpp"

#include <SFML/Graphics.hpp>
#include <rpp/rpp.hpp>

static SnakeBody generate_initial_snake_body()
{
    // tail to head
    return SnakeBody{ Coordinates{1,1}, Coordinates{2,1}, Coordinates{3,1}, Coordinates{4,1} };
}

static Coordinates generate_initial_apple()
{
    return Coordinates{ 3,5 };
}

static int wrap_coordinate(int value, int max_value)
{
    return value < 0 ? max_value : value > max_value ? 0 : value;
}

static SnakeBody&& move_snake(SnakeBody&& body, const Direction& direction)
{
    auto head = body.back();
    std::ranges::rotate(body, body.begin() + 1);

    head.x += direction.x;
    head.x = wrap_coordinate(head.x, s_columns_count);

    head.y += direction.y;
    head.y = wrap_coordinate(head.y, s_rows_count);

    body.back() = head;

    return std::move(body);
}

static bool is_snake_eat_self(const SnakeBody& body)
{
    const auto& head = body.back();

    return std::find(body.cbegin(), body.cend() - 1, head) == body.cend() - 1;
}

static Coordinates&& update_apple_position_if_eat(Coordinates&& apple_position, const SnakeBody& snake)
{
    return std::move(apple_position);
}

rpp::dynamic_observable<sf::RectangleShape> get_shapes_to_draw(const rpp::dynamic_observable<CustomEvent>& events)
{
    const auto key_event = events.filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
                                 .map([](const CustomEvent& ev) { return std::get<sf::Event>(ev); })
                                 .filter([](const sf::Event& event) { return event.type == sf::Event::KeyPressed; })
                                 .map([](const sf::Event& event) { return event.key; });

    static const std::map<sf::Keyboard::Key, Direction> s_key_to_direction
    {
        {sf::Keyboard::Key::Right, { 1,  0}},
        {sf::Keyboard::Key::Left,  {-1,  0}},
        {sf::Keyboard::Key::Down,  { 0,  1}},
        {sf::Keyboard::Key::Up,    { 0, -1}},
    };

    auto direction = key_event.filter([](const sf::Event::KeyEvent& key_event)
                              {
                                  return !key_event.alt && !key_event.control && !key_event.shift && !key_event.system;
                              })
                              .map([](const sf::Event::KeyEvent& event)-> std::optional<Direction>
                              {
                                  const auto itr = s_key_to_direction.find(event.code);
                                  if (itr != s_key_to_direction.cend())
                                      return itr->second;
                                  return std::nullopt;
                              })
                              .filter([](const auto& optional) { return optional.has_value(); })
                              .map([](const auto&    optional) { return optional.value(); })
                              .start_with(s_key_to_direction.at(sf::Keyboard::Key::Right));

    auto snake = rpp::source::interval(std::chrono::milliseconds{200}, g_run_loop)
                 .with_latest_from([](const auto&, const Direction& direction) { return direction; }, std::move(direction))
                 .scan(generate_initial_snake_body(), &move_snake)
                 .publish()
                 .ref_count();

    auto apples = snake.scan(generate_initial_apple(), update_apple_position_if_eat);

    return get_presents_stream(events)
           .filter([](const PresentEvent&  ev) { return ev.is_begin; })
           .with_latest_from([](const auto&, const SnakeBody& snake_body) { return snake_body; }, std::move(snake))
           .switch_map([](const SnakeBody& body) { return rpp::source::from_iterable(body); })
           .map([](const Coordinates&      coords) { return get_rectangle_at(coords, sf::Color::Red); });
}
