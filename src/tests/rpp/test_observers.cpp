//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <snitch/snitch.hpp>
#include <rpp/observers.hpp>

#include <vector>

TEST_CASE("lambda observer works properly as base observer")
{
    std::vector<int> on_next_vals{};
    size_t           on_error{};
    size_t           on_completed{};
    auto             observer = rpp::make_lambda_observer<int>([&](int v) { on_next_vals.push_back(v); },
                                                               [&](const std::exception_ptr&) { ++on_error; },
                                                               [&]() { ++on_completed; });

    auto test_observer = [&](const auto& obs)
    {
        obs.on_next(1);
        CHECK(on_next_vals == std::vector{1});

        obs.on_next(2);
        CHECK(on_next_vals == std::vector{1, 2});

        obs.on_error(std::exception_ptr{});
        CHECK(on_error == 1);

        obs.on_completed();
        CHECK(on_completed == 1);
    };

    SECTION("lambda observer obtains callbacks") {
        static_assert(!std::is_copy_constructible_v<decltype(observer)>, "lambda observer shouldn't be copy constructible");
        test_observer(observer);
    }

    SECTION("lambda observer obtains callbacks via cast to dynamic_observer") {
        auto dynamic_observer = std::move(observer).as_dynamic();
        static_assert(std::is_copy_constructible_v<decltype(dynamic_observer)>, "dynamic observer should be copy constructible");
        test_observer(dynamic_observer);
    }
}
