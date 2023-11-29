#include "canvas.hpp"
#include "snake.hpp"
#include "utils.hpp"

#include <rpp/rpp.hpp>
#include <SFML/Graphics.hpp>

#include <variant>

static auto get_events_observable(sf::RenderWindow& window)
{
    return rpp::source::create<CustomEvent>([&window](auto&& observer)
    {
        auto worker = g_run_loop.create_worker();

        worker.schedule([frame_number = size_t{}, ev = sf::Event{}, &window](const auto& obs) mutable->rpp::schedulers::optional_delay_from_now
        {
            // GCC compile issue =C
            rpp::schedulers::optional_delay_from_now res{};
            if (!window.isOpen())
                return res;
            
            // indicate new frame
            obs.on_next(PresentEvent{ frame_number++ });
        
            while (window.pollEvent(ev))
                obs.on_next(ev);

            return rpp::schedulers::delay_from_now{};
        }, std::forward<decltype(observer)>(observer));
    });
}

#include <iostream>

int main()
{
    auto window_size = get_window_size(s_rows_count, s_columns_count);
    sf::RenderWindow window(sf::VideoMode(window_size.x, window_size.y), "Snake");

    const auto events = get_events_observable(window) | rpp::ops::publish();
    const auto presents = get_presents_stream(events);

    auto start = rpp::schedulers::clock_type::now();
    presents.subscribe([&window, &start](const PresentEvent& p)
    {
        window.display();
        std::cout << "FPS: " << ((static_cast<double>(p.frame_number) / static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(rpp::schedulers::clock_type::now() - start).count()))*1000000000.0) << std::endl;
        window.clear(sf::Color{0, 128, 0});
    });

    get_shapes_to_draw(events).subscribe([&window](const auto& shape)
    {
        window.draw(shape);
    });

    const auto root_subscription =events
                                  | rpp::ops::ref_count()
                                  | rpp::ops::filter([](const CustomEvent& ev) { return std::holds_alternative<sf::Event>(ev); })
                                  | rpp::ops::filter([](const CustomEvent& ev)
                                  {
                                      return std::get<sf::Event>(ev).type == sf::Event::Closed;
                                  })
                                  | rpp::ops::take(1)
                                  | rpp::ops::subscribe_with_disposable([&](const auto&)
                                  {
                                      window.close();
                                  });

    // this one will be blocking call and it will unblock when close requested
    while (!root_subscription.is_disposed())
    {
        while (g_run_loop.is_any_ready_schedulable())
            g_run_loop.dispatch();
    }

    return EXIT_SUCCESS;
}