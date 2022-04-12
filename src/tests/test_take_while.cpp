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
#include "rpp/sources/create.h"

#include <rpp/operators/take_while.h>
#include <catch2/catch_test_macros.hpp>

SCENARIO("take_while filters values")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable")
    {
        auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            int v{};
            while (sub.is_subscribed())
                sub.on_next(v++);
        });
        WHEN("subscribe on it with take_while")
        {
            obs.take_while([](int val)
            {
                return val <= 5;
            }).subscribe(mock);

            THEN("only items before false obtained")
            {
                CHECK(mock.get_received_values() == std::vector{ 0, 1, 2, 3, 4, 5 });
            }
        }
    }
}
