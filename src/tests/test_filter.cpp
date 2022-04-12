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

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/filter.h>
#include <rpp/sources/create.h>

#include "mock_observer.h"

SCENARIO("Filter provides only satisfied items")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable")
    {
        auto observable = rpp::source::create<int>([](const auto& sub)
        {
            for (int i = 1; i < 5; ++i)
                sub.on_next(i);
        });

        WHEN("subscribe on filtered observable")
        {
            observable.filter([](int v)
            {
                return v % 2 == 0;
            }).subscribe(mock);

            THEN("obtained only satisfied items")
            {
                CHECK(mock.get_received_values() == std::vector<int>{2, 4});
            }
        }
    }
}
