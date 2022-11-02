#include <rpp/rpp.hpp>

#include <iostream>

/**
 * \example timeout.cpp
 **/
int main()
{
    //! [timeout]
    rpp::subjects::publish_subject<int> subj{};
    subj.get_observable()
        .timeout(std::chrono::milliseconds{450}, rpp::schedulers::new_thread{})
        .subscribe([](int                v) { std::cout << "new value " << v << std::endl; },
                   [](std::exception_ptr err)
                   {
                       try
                       {
                           std::rethrow_exception(err);
                       }
                       catch (const std::exception& exc)
                       {
                           std::cout << "ERR: " << exc.what() << std::endl;
                       }
                   },
                   []() { std::cout << "completed" << std::endl; });
    for (int i = 0; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{i * 100});
        subj.get_subscriber().on_next(i);
    }
    
    // Output:
    // new value 0
    // new value 1
    // new value 2
    // new value 3
    // new value 4
    // ERR : Timeout reached
    //! [timeout]
    return 0;
}
