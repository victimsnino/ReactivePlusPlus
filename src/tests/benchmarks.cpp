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
#include <rpp/observers/specific_observer.h>

#include <array>

template<template<typename...> typename TObserver>
auto MakeObserver()
{
    std::array<int, 1> v{};

    return TObserver{[v](int)
    {
        [[maybe_unused]] const auto& t = v;
    }};
}

auto MakeSpecificObservable()
{
    std::array<int, 1> v{};

    return rpp::make_specific_observable<int>([v](const auto& sub)
    {
        sub.on_next(static_cast<int>(v.size()));
    });
}

auto MakeDynamicObservable()
{
    std::array<int, 1> v{};

    return rpp::dynamic_observable<int>{[v](const auto& sub)
    {
        sub.on_next(static_cast<int>(v.size()));
    }};
}

SCENARIO("Base benchmarks", "[benchmark]")
{
    BENCHMARK("Specific observable construction")
    {
        return MakeSpecificObservable();
    };

    BENCHMARK("Dynamic observable construction")
    {
        return MakeDynamicObservable();
    };

    //BENCHMARK("Specific observable construction as_dynamic")
    //{
    //    return MakeSpecificObservable().as_dynamic();
    //};

    BENCHMARK("Specific observer construction")
    {
        return MakeObserver<rpp::specific_observer>();
    };

    BENCHMARK("Dynamic observer construction")
    {
        return MakeObserver<rpp::dynamic_observer>();
    };

    BENCHMARK("Specific observer construction as_dynamic")
    {
        return MakeObserver<rpp::specific_observer>().as_dynamic();
    };

    auto specific_observer = MakeObserver<rpp::specific_observer>();
    auto dynamic_observer  = MakeObserver<rpp::dynamic_observer>();

    BENCHMARK("Specific observer OnNext", i)
    {
        specific_observer.on_next(i);
        return i;
    };

    BENCHMARK("Dynamic observer OnNext", i)
    {
        dynamic_observer.on_next(i);
        return i;
    };

    BENCHMARK("Make subsriber")
    {
        return rpp::specific_subscriber{[](const int&){}};
    };

    auto sub = rpp::specific_subscriber{ [](const int&) {} };

    BENCHMARK("Make copy of subscriber")
    {
        auto second = sub;
        return second;
    };

    BENCHMARK("Transform subsriber to dynamic")
    {
        return sub.as_dynamic();
    };

    auto validate_observable = [](auto observable, const std::string& observable_prefix)
    {
        auto validate_with_observer = [&](const auto& observer, const std::string& observer_prefix)
        {
            BENCHMARK(observable_prefix+ " observable subscribe " + observer_prefix +" observer")
            {
                return observable.subscribe(observer);
            };
            BENCHMARK(observable_prefix + " observable lift " + observer_prefix +" observer")
            {
                auto res_observable = observable.template lift<int>([](const auto& v) { return v; });
                return res_observable.subscribe(observer);
            };
        };
        validate_with_observer(MakeObserver<rpp::specific_observer>(), "specific");
        validate_with_observer(MakeObserver<rpp::dynamic_observer>(), "dynamic");
    };

    validate_observable(MakeSpecificObservable(), "Specific");
    validate_observable(MakeDynamicObservable(), "Dynamic");
}

SCENARIO("Misc benchmarks", "[misc_benchmark]")
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
