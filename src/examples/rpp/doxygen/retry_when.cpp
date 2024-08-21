#include <rpp/rpp.hpp>

#include <iostream>
#include <string>

/**
 * @example retry_when.cpp
 **/

int main()
{
    //! [retry_when delay]
    size_t retry_count = 0;
    rpp::source::create<std::string>([&retry_count](const auto& sub) {
        if (++retry_count != 4)
        {
            sub.on_error({});
        }
        else
        {
            sub.on_next(std::string{"success"});
            sub.on_completed();
        }
    })
        | rpp::operators::retry_when([](const std::exception_ptr&) {
              return rpp::source::timer(std::chrono::seconds{5}, rpp::schedulers::current_thread{});
          })
        | rpp::operators::subscribe([](const std::string& v) { std::cout << v << std::endl; });
    // Source observable is resubscribed after 5 seconds on each error emission
    //! [retry_when delay]

    //! [retry_when]
    retry_count = 0;
    rpp::source::create<std::string>([&retry_count](const auto& sub) {
        if (++retry_count != 4)
        {
            sub.on_error({});
        }
        else
        {
            sub.on_next(std::string{"success"});
            sub.on_completed();
        }
    })
        | rpp::operators::retry_when([](const std::exception_ptr& ep) {
              try
              {
                  std::rethrow_exception(ep);
              }
              catch (const std::runtime_error&)
              {
                  return rpp::source::timer(std::chrono::seconds{5}, rpp::schedulers::current_thread{});
              }
              catch (...)
              {
                  throw;
              }
          })
        | rpp::operators::subscribe([](const std::string& v) { std::cout << v << std::endl; });
    // Source observable is resubscribed after 5 seconds only on particular error emissions
    //! [retry_when]
    return 0;
}
