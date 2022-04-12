#include <rpp/rpp.h>

#include <iostream>

int main()
{
    auto observable = rpp::source::create<char>([](const auto& sub)
                      {
                          while (sub.is_subscribed())
                          {
                              sub.on_next(std::getchar());
                          }
                      })
                      .filter(std::not_fn(&::isdigit))
                      .map(&::toupper);

    observable.subscribe([](char v)
    {
        std::cout << v;
    });
    return 0;
}
