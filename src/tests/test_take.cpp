// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "mock_observer.h"
#include "rpp/sources.h"

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/take.h>

SCENARIO("take limits count of items")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable")
    {
        auto obs = rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 0; i < 10; ++i)
            {
                auto new_sub = sub; // send it to copy to test for shared
                new_sub.on_next(i);
            }
        });
        WHEN("subscribe on it with take")
        {
            constexpr size_t count   = 5;
            auto             new_obs = obs.take(count);
            new_obs.subscribe(mock);
            THEN("only limited amount of items provided")
            {
                CHECK(mock.get_received_values() == std::vector{ 0, 1, 2, 3, 4 });
                CHECK(mock.get_on_completed_count() == 1);
            }
            auto mock_2 = mock_observer<int>{};
            new_obs.subscribe(mock_2);
            AND_THEN("second subscription see same amount of items")
            {
                CHECK(mock_2.get_received_values() == std::vector{ 0, 1, 2, 3, 4 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    GIVEN("observable with check for subscription")
    {
        size_t loop_count = 0;
        auto obs          = rpp::source::create<int>([&](const auto& sub)
        {
            while (sub.is_subscribed())
            {
                sub.on_next(1);
                ++loop_count;
            }
        });
        WHEN("subscribe on it with take")
        {
            constexpr size_t limit_count = 2;
            obs.take(limit_count).subscribe(mock);
            THEN("it immediately unsubscribed when condition meets")
            {
                CHECK(mock.get_total_on_next_count() == limit_count);
                CHECK(loop_count == limit_count);
            }
        }
    }
}
