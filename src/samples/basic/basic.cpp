#include <rpp/rpp.h>

#include <functional>
#include <iostream>

int main()
{
    auto observable = rpp::source::create<char>([](const auto& sub)
                      {
                          while (sub.is_subscribed())
                              sub.on_next(std::getchar());
                      })
                      .take_while([](char v) { return v != '0'; })
                      .filter(std::not_fn(&::isdigit))
                      .map(&::toupper);

    observable.subscribe([](char v)
    {
        std::cout << v;
    });
    return 0;
}
