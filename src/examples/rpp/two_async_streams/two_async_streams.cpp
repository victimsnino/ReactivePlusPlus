#include <rpp/rpp.hpp>

#include <functional>
#include <iostream>
#include <sstream>

rpp::schedulers::time_point start{};

template<typename T>
std::string format_message(T data) {
    std::stringstream ss{};
    ss << "[" 
              << std::this_thread::get_id() 
              << "][" << std::chrono::duration_cast<std::chrono::milliseconds>(rpp::schedulers::clock_type::now() - start).count() 
              << "] Message: " 
              << data
              << std::endl;
    return ss.str();
}

int main() // NOLINT(bugprone-exception-escape)
{
    auto keyboard_events = rpp::source::from_callable(&::getchar)
                         | rpp::operators::repeat()
                         | rpp::operators::take_while([](char v) { return v != '0'; })
                         | rpp::operators::filter(std::not_fn(&::isdigit))
                         | rpp::operators::map(&::toupper)
                         | rpp::operators::map(format_message<char>);

    return 0;
}