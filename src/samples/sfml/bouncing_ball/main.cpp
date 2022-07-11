#include <iostream>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <rpp/rpp.hpp>

#include <filesystem>

std::string get_fonth_path()
{
#if defined(_WIN32)
    return "C:/Windows/Fonts/arial.ttf";
#elif defined(_APPLE_) && defined(_MACH_)
    return "/Library/Fonts/Arial.ttf";
#elif defined(linux) || defined(__linux)
    return "/usr/share/fonts/truetype/freefont/FreeSans.ttf";
#else
    return "";
#endif
}

auto load_font()
{
    sf::Font font{};

    if (font.loadFromFile(get_fonth_path()))
        return font;

    throw std::runtime_error{"Can't find font!"};
}

template <typename T>
std::string to_string_with_presision(const T a_value, const int n = 1)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

auto prepare_ball_shape()
{
    sf::CircleShape shape(20.f);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(sf::Vector2f{20, 20});

    return shape;
}

auto prepare_text_shape(const sf::Font& font)
{
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::Red);
    text.setStyle(sf::Text::Bold);
    text.setString("(000.0, 000.0)");
    text.setOrigin(sf::Vector2f{text.getLocalBounds().width / 2, text.getLocalBounds().height / 2});

    return text;
}

uint8_t deduce_color_smoothly(size_t color_value)
{
    const uint8_t rest = color_value % 255;
    const uint32_t overflow_count = color_value / 255;
    if (overflow_count % 2)
        return uint8_t{255} - rest;
    return rest;
}

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
            sub.on_next(PresentEvent{true, frame_number});

            while (window.pollEvent(ev))
                sub.on_next(ev);

            sub.on_next(PresentEvent{false, frame_number++});
        }
    });
}

template<typename State>
void handle_presents(const rpp::specific_observable<CustomEvent, State>& events_observable, sf::RenderWindow& window)
{
    auto presents = events_observable.filter([](const CustomEvent& ev)
                                     {
                                         return std::holds_alternative<PresentEvent>(ev);
                                     })
                                     .map([](const CustomEvent& ev)
                                     {
                                         return std::get<PresentEvent>(ev);
                                     });

    presents.filter([](const PresentEvent& ev)
            {
                return ev.is_begin;
            })
            .subscribe([&window](const auto& ev)
            {
                window.clear(sf::Color{deduce_color_smoothly(ev.frame_number * 1), 
                                       deduce_color_smoothly(ev.frame_number * 2),
                                       deduce_color_smoothly(ev.frame_number * 3)});
            });

    presents.filter([](const PresentEvent& ev)
            {
                return !ev.is_begin;
            })
            .subscribe([&window](const auto&)
            {
                window.display();
            });
}

template<typename State>
void handle_ball_position(const rpp::specific_observable<CustomEvent, State>& events_observable)
{
    auto end_presents = events_observable
                    .filter([](const CustomEvent& ev)
                    {
                        return std::holds_alternative<PresentEvent>(ev);
                    })
                    .filter([](const CustomEvent& ev)
                    {
                        return std::get<PresentEvent>(ev).is_begin == false;
                    });
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{680, 680}), "Bouncing Ball");

    auto font       = load_font();
    auto ball_shape = prepare_ball_shape();
    auto text_shape = prepare_text_shape(font);

    float x       = 50;
    float y       = 150;
    float x_speed = 1.f;
    float y_speed = 1.f;

    auto events = get_events_observable(window).publish();

    // clear on begin, display on end
    handle_presents(events, window);

    // this one will be blocking call and it will unblock when close requested
    events.ref_count()
          .filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
          .filter([](const CustomEvent& ev) { return std::get<sf::Event>(ev).type == sf::Event::Closed; })
          .take(1)
          .subscribe([&](const auto&)
          {
              window.close();
          });
    
   /* while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        x += x_speed;
        y += y_speed;
        if (x < 0 + ball_shape.getRadius() || window.getSize().x - ball_shape.getRadius() < x)
        {
            x_speed *= -1;
        }

        if (y < 0 + ball_shape.getRadius() || window.getSize().y - ball_shape.getRadius() < y)
        {
            y_speed *= -1;
        }

        ball_shape.setPosition(sf::Vector2f{x, y});
        text_shape.setPosition(sf::Vector2f{x, y - ball_shape.getRadius()});
        text_shape.setString("(" + to_string_with_presision(x) + ", " + to_string_with_presision(y) + ")");
 
        window.clear(sf::Color::White);
        window.draw(ball_shape);
        window.draw(text_shape);
        window.display();
    }*/
 
    return EXIT_SUCCESS;
}