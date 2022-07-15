#include <SFML/Graphics.hpp>
#include <rpp/rpp.hpp>

#include <iostream>

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
            sf::Event ev{};

            size_t frame_number{};
            while (window.isOpen() && sub.is_subscribed())
            {
                sub.on_next(PresentEvent{ true, frame_number });

                while (window.pollEvent(ev))
                    sub.on_next(ev);

                sub.on_next(PresentEvent{ false, frame_number++ });
            }
        });
}

constexpr sf::Vector2f g_rectangle_size{10, 10};

sf::RectangleShape get_rectangle_at(sf::Vector2f location, sf::Color color)
{
    sf::RectangleShape box;
    box.setSize(g_rectangle_size);
    box.setPosition(location);
    box.setFillColor(color);
    return box;

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
                window.clear(sf::Color{0, 0, 0});
            });
}

void game_logic(const rpp::dynamic_observable<CustomEvent>& events, sf::RenderWindow& window)
{
    const auto sf_events = events.filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
                                 .map([](const CustomEvent& ev) { return std::get<sf::Event>(ev); });

    const auto key_event = sf_events.filter([](const sf::Event& event) { return event.type == sf::Event::KeyPressed; })
                                    .map([](const sf::Event& event) { return event.key; });

    struct Coordinates
    {
        int x{};
        int y{};
    };
    using Direction = Coordinates;

    const std::map<sf::Keyboard::Key, Direction> key_to_direction{
        {sf::Keyboard::Key::Left, {-1, 0}},
        {sf::Keyboard::Key::Right, {1, 0}},
        {sf::Keyboard::Key::Up, {0, 1}},
        {sf::Keyboard::Key::Up, {0, -1}},
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
                                    .filter(&std::optional<Coordinates>::has_value)
                                    .map([](const auto& optional) { return optional.value(); })
                                    .start_with();
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{680, 680}), "Snake");

    const auto events = get_events_observable(window).publish();
    const auto presents = get_presents_stream(events);

    display_screen_and_clear_on_present_begin(presents, window);

    game_logic(events, window);

    // this one will be blocking call and it will unblock when close requested
    events.ref_count()
          .filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
          .filter([](const CustomEvent& ev) { return std::get<sf::Event>(ev).type == sf::Event::Closed; })
          .take(1)
          .subscribe([&](const auto&)
          {
              window.close();
          });

    return EXIT_SUCCESS;
}
