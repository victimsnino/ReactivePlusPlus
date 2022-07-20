#include <SFML/Graphics.hpp>
#include <rpp/rpp.hpp>

#include "snake.hpp"
#include "canvas.hpp"

#include <iostream>

static rpp::schedulers::run_loop s_run_loop{};

struct PresentEvent
{
    bool is_begin{};
    size_t frame_number{};
};

using CustomEvent = std::variant<PresentEvent, sf::Event>;

auto get_events_observable(sf::RenderWindow& window)
{
    return rpp::source::create<CustomEvent>([&window](const auto& sub)
    {
        auto worker = s_run_loop.create_worker(sub.get_subscription());

        worker.schedule([frame_number = size_t{}, &window, sub]() mutable -> rpp::schedulers::optional_duration
        {
            sf::Event ev{};
            if (!window.isOpen())
                return {};

            sub.on_next(PresentEvent{true, frame_number});

            while (window.pollEvent(ev))
                sub.on_next(ev);

            sub.on_next(PresentEvent{false, frame_number++});
            return rpp::schedulers::duration{};
        });
    });
}


auto get_presents_stream(const auto& events)
{
    return events.filter([](const CustomEvent& ev) { return std::holds_alternative<PresentEvent>(ev); })
                 .map([](const CustomEvent&    ev) { return std::get<PresentEvent>(ev); });
}

void display_screen_and_clear_on_present_begin(const auto& presents, sf::RenderWindow& window)
{
    presents.filter([](const PresentEvent&  ev) { return ev.is_begin; })
            .subscribe([&window](const auto&)
            {
                window.display();
                window.clear(sf::Color{0, 128, 0});
            });
}

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

void game_logic(const rpp::dynamic_observable<CustomEvent>& events, sf::RenderWindow& window)
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

    auto snake = rpp::source::interval(std::chrono::milliseconds{200}, s_run_loop)
                 .with_latest_from([](const auto&, const Direction& direction) { return direction; }, direction)
                 .scan(generate_initial_snake_body(), &move_snake);

    get_presents_stream(events).filter([](const PresentEvent&  ev) { return ev.is_begin; })
                               .with_latest_from([](const auto&, const SnakeBody& snake_body) { return snake_body; }, std::move(snake))
                               .flat_map([](const SnakeBody& body){return rpp::source::from_iterable(body);})
                               .map([](const Coordinates& coords){return get_rectangle_at(coords, sf::Color::Red); })
                               .subscribe([&window](const sf::RectangleShape& rectangle)
                               {
                                   window.draw(rectangle);
                               });
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(get_window_size(s_rows_count, s_columns_count)), "Snake");

    const auto events = get_events_observable(window).publish();
    const auto presents = get_presents_stream(events);

    display_screen_and_clear_on_present_begin(presents, window);

    game_logic(events, window);

    const auto root_subscription = events.ref_count()
                                         .filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
                                         .filter([](const CustomEvent& ev) { return std::get<sf::Event>(ev).type == sf::Event::Closed; })
                                         .take(1)
                                         .subscribe([&](const auto&)
                                         {
                                             window.close();
                                         });

    // this one will be blocking call and it will unblock when close requested
    while(root_subscription.is_subscribed())
    {
        while (s_run_loop.is_any_ready_schedulable())
            s_run_loop.dispatch();
    }

    return EXIT_SUCCESS;
}
