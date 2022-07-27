#include "canvas.hpp"
#include "snake.hpp"
#include "utils.hpp"

#include <rpp/rpp.hpp>
#include <SFML/Graphics.hpp>

#include <variant>

static auto get_events_observable(sf::RenderWindow& window)
{
    return rpp::source::create<CustomEvent>([&window](const auto& sub)
        {
            auto worker = g_run_loop.create_worker(sub.get_subscription());

            worker.schedule([frame_number = size_t{}, &window, sub]() mutable->rpp::schedulers::optional_duration
            {
                sf::Event ev{};
                if (!window.isOpen())
                    return {};

                sub.on_next(PresentEvent{ true, frame_number });

                while (window.pollEvent(ev))
                    sub.on_next(ev);

                sub.on_next(PresentEvent{ false, frame_number++ });
                return rpp::schedulers::duration{};
            });
        });
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

int main()
{
    sf::RenderWindow window(sf::VideoMode(get_window_size(s_rows_count, s_columns_count)), "Snake");

    const auto events = get_events_observable(window).publish();
    const auto presents = get_presents_stream(events);

    display_screen_and_clear_on_present_begin(presents, window);

    get_shapes_to_draw(events).subscribe([&window](const auto& shape)
    {
        window.draw(shape);
    });

    const auto root_subscription =events
                                  .ref_count()
                                  .filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
                                  .filter([](const CustomEvent& ev)
                                  {
                                      return std::get<sf::Event>(ev).type == sf::Event::Closed;
                                  })
                                  .take(1)
                                  .subscribe([&](const auto&)
                                  {
                                      window.close();
                                  });

    // this one will be blocking call and it will unblock when close requested
    while (root_subscription.is_subscribed())
    {
        while (g_run_loop.is_any_ready_schedulable())
            g_run_loop.dispatch();
    }

    return EXIT_SUCCESS;
}
