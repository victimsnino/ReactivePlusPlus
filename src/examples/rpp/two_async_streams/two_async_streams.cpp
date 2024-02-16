#include <rpp/rpp.hpp>

#include <functional>
#include <iostream>
#include <sstream>

rpp::schedulers::time_point start{};

template<typename T>
std::string format_message(T data)
{
    std::stringstream ss{};
    ss << "["
       << std::this_thread::get_id()
       << "][" << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count()
       << "ms] Message: "
       << data;
    return ss.str();
}

int main() // NOLINT(bugprone-exception-escape)
{
    auto raw_keyboard_events = rpp::source::from_callable(&::getchar)
                             | rpp::operators::repeat()
                             | rpp::operators::subscribe_on(rpp::schedulers::new_thread{})
                             | rpp::operators::publish()
                             | rpp::operators::ref_count();

    auto termination = raw_keyboard_events
                     | rpp::operators::filter([](char v) { return v == '0'; });

    auto chars = raw_keyboard_events
               | rpp::operators::filter([](char v) { return !std::isdigit(v) && v != '\n'; })
               | rpp::operators::map(&::toupper)
               | rpp::operators::map(format_message<char>);


    start = rpp::schedulers::clock_type::now();
    std::cout << format_message("main thread") << std::endl;

    rpp::source::interval(std::chrono::seconds{1}, rpp::schedulers::new_thread{})
        | rpp::operators::map([](size_t i) { return format_message(std::string{"counter "} + std::to_string(i)); })
        | rpp::operators::merge_with(chars)
        | rpp::operators::observe_on(rpp::schedulers::new_thread{})
        | rpp::operators::take_until(termination)
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe([](const std::string& event) {
              std::cout << event << std::endl;
          });

    std::cout << "EXIT" << std::endl;

    return 0;
}