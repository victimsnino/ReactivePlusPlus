// PORT OF https://github.com/ReactiveX/RxCpp/blob/main/Rx/v2/examples/linesfrombytes/main.cpp

#include <rpp/rpp.hpp>

#include <iostream>
#include <random>
#include <regex>
#include <vector>

int main() // NOLINT(bugprone-exception-escape)
{
    std::random_device              rd; // non-deterministic generator
    std::mt19937                    gen(rd());
    std::uniform_int_distribution<> dist(4, 18);

    // for testing purposes, produce byte stream that from lines of text
    auto bytes = rpp::source::just(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
               | rpp::operators::flat_map([&](int i) {
                     auto body  = rpp::source::just((uint8_t)('A' + i)) | rpp::operators::repeat(dist(gen));
                     auto delim = rpp::source::just((uint8_t)'\r');
                     return rpp::source::concat(body, delim);
                 })
               | rpp::operators::window(17) 
               | rpp::operators::flat_map([](auto&& w) {
                     return std::forward<decltype(w)>(w) | rpp::operators::reduce(std::vector<uint8_t>(), [](std::vector<uint8_t> v, uint8_t b) {
                                v.push_back(b);
                                return v;
                            });
                 })
               | rpp::operators::tap([](const std::vector<uint8_t>& v) {
                     // print input packet of bytes
                     std::copy(v.begin(), v.end(), std::ostream_iterator<long>(std::cout, " "));
                     std::cout << std::endl;
                 });

    //
    // recover lines of text from byte stream
    //

    auto removespaces = [](std::string s) {
        s.erase(std::remove_if(s.begin(), s.end(), &isspace), s.end());
        return s;
    };

    // create strings split on \r
    auto strings = bytes | rpp::operators::map([](std::vector<uint8_t> v) {
                       std::string                s(v.begin(), v.end());
                       std::regex                 delim(R"/(\r)/");
                       std::cregex_token_iterator cursor(&s[0], &s[0] + s.size(), delim, {-1, 0});
                       std::cregex_token_iterator end;
                       std::vector<std::string>        splits(cursor, end);
                       return rpp::source::from_iterable(std::move(splits));
                   })
                 | rpp::operators::concat()
                 | rpp::operators::filter([](const std::string& s) {
                       return !s.empty();
                   })
                 | rpp::operators::publish() | rpp::operators::ref_count();

    // filter to last string in each line
    auto closes = strings | rpp::operators::filter([](const std::string& s) {
                      return s.back() == '\r';
                  })
                | rpp::operators::map([](const std::string&) { return 0; });

    // group strings by line
    auto linewindows = strings | rpp::operators::window_toggle(closes | rpp::operators::start_with(0), [=](int) { return closes; });

    // reduce the strings for a line into one string
    auto lines = linewindows | rpp::operators::flat_map([&](auto&& w) {
                     return std::forward<decltype(w)>(w) | rpp::operators::start_with(std::string{""}) | rpp::operators::reduce(std::plus<std::string>{}) | rpp::operators::map(removespaces);
                 });

    // print result
    lines | rpp::operators::subscribe([](const std::string& s) { std::cout << s << std::endl; });
    return 0;
}