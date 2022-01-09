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
#include <catch2/benchmark/catch_benchmark.hpp>

#include <rpp/observable.h>
#include <rpp/observer.h>
#include <rpp/subscriber.h>

#include <array>

SCENARIO("Benchmark observer", "[benchmark]")
{
    auto make_observer_and_observable = []()
    {
        std::array<int, 100> v{};
        auto                 observer   = rpp::observer{[v](int                           ) {}};
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

    const auto tuple      = make_observer_and_observable();
    const auto observer   = std::get<0>(tuple);
    const auto observable = std::get<1>(tuple);

    BENCHMARK("Subscribe")
    {
        observable.subscribe(observer);
    };

    BENCHMARK("OnNext", i)
    {
        observer.on_next(i);
    };
}

SCENARIO("Misc benchmarks", "[benchmark]")
{
    std::array<int, 100> arr{1};
    auto                 empty_lambda = [arr](int i) { return i * arr[0]; };

    BENCHMARK("Raw call to lambda", i) { return empty_lambda(i); };

    std::function as_function = empty_lambda;
    BENCHMARK("Call to lambda via std::function", i) { return as_function(i); };

    BENCHMARK("Copy  lambda") { return empty_lambda; };
    BENCHMARK("Copy  std::function") { return as_function; };

    BENCHMARK("Make shared  copy of lambda"){return std::make_shared<std::remove_reference_t<decltype(empty_lambda)>>(empty_lambda); };
}
