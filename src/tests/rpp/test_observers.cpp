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

TEST_CASE("lambda observer as base observer")
{
    SECTION("lambda observer")
    {
        std::vector<int> on_next_vals{};
        size_t on_error{};
        size_t on_completed{};
        auto observer = rpp::make_lambda_observer<int>([&](int v){ on_next_vals.push_back(v); }, [&](const std::exception_ptr& ){ ++on_error; }, [&](){ ++on_completed; });

        auto test_observer = [&](const auto& obs)
        {
            obs.on_next(1);
            obs.on_next(2);
            obs.on_error(std::exception_ptr{});
            obs.on_completed();
            SECTION("lambdas obtain callbacks")
            {
                CHECK(on_next_vals == std::vector{1,2});
                CHECK(on_error == 1);
                CHECK(on_completed == 1);
            }
        };

        SECTION("call callbacks on it")
        {
            test_observer(observer);
        }

        SECTION("call callbacks via dynamic_observer on it")
        {
            test_observer(std::move(observer).as_dynamic());
        }
    }
}