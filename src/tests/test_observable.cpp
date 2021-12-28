// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
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
#include <catch2/benchmark/catch_benchmark.hpp>

#include <rpp/observable.h>
#include <rpp/observer.h>
#include <rpp/subscriber.h>

#include <array>

SCENARIO("Observable should be subscribable")
{
    GIVEN("observer and observable of same type")
    {
        size_t     on_next_called_count = 0;
        const auto observer             = rpp::observer{[&](int) { ++on_next_called_count; }};

        size_t     on_subscribe_called_count = 0;
        const auto observable                = rpp::observable{[&](const rpp::subscriber<int>& sub)
        {
            ++on_subscribe_called_count;
            sub.on_next(123);
        }};

        WHEN("subscribe called for observble")
        {
            observable.subscribe(observer);

            THEN("OnSubscribe lambda called once")
            {
                CHECK(on_subscribe_called_count == 1);
            }
            AND_THEN("on_next lambda called once")
            {
                CHECK(on_next_called_count == 1);
            }
        }
    }
}

SCENARIO("Benchmark observer")
{
    auto make_observer_and_observable = []()
    {
        std::array<int, 100> v{};
        auto                 observer   = rpp::observer{[v](int) {}};
        auto                 observable = rpp::observable{[v](const rpp::subscriber<int>& sub)
        {
            sub.on_next(123);
        }};
        return std::tuple{observer, observable};
    };

    BENCHMARK("Construction")
    {
        auto [observer, observable] = make_observer_and_observable();

        observable.subscribe(observer);
    };

    auto [observer, observable] = make_observer_and_observable();

    BENCHMARK("Subscribe")
    {
        observable.subscribe(observer);
    };
}
