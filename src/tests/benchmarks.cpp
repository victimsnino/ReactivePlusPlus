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
#include <rpp/observables.h>
#include <rpp/sources.h>
#include <rpp/observables/dynamic_observable.h>
#include <rpp/observers/specific_observer.h>
#include <rpp/operators/map.h>

#include <array>

template<template<typename...> typename TObserver>
auto MakeObserver()
{
    return TObserver{[](int) { }};
}

auto MakeSpecificObservable()
{
    return rpp::observable::create<int>([](const auto& sub) { });
}

auto MakeDynamicObservable()
{
    return rpp::dynamic_observable<int>{[](const auto& sub) { }};
}

TEST_CASE("Observable construction", "[benchmark]")
{
    BENCHMARK("Specific observable construction")
    {
        return MakeSpecificObservable();
    };

    BENCHMARK("Dynamic observable construction")
    {
        return MakeDynamicObservable();
    };

    BENCHMARK("Specific observable construction + as_dynamic")
    {
        return MakeSpecificObservable().as_dynamic();
    };

}

TEST_CASE("Observable subscribe #2", "[benchmark]")
{
    auto specific = MakeSpecificObservable();
    BENCHMARK("Specific observable subscribe lambda")
    {
        return specific.subscribe([](const auto&) {});
    };

    auto dynamic = MakeDynamicObservable();
    BENCHMARK("Dynamic observable subscribe lambda")
    {
        return dynamic.subscribe([](const auto&) {});
    };

    BENCHMARK("Specific observable subscribe lambda without subscription")
    {
        return specific.subscribe([](const auto&) {});
    };

    BENCHMARK("Dynamic observable subscribe lambda without subscription")
    {
        return dynamic.subscribe([](const auto&) {});
    };

    auto subscriber = rpp::specific_subscriber([](const int&){});
    BENCHMARK("Specific observable subscribe specific subscriber")
    {
        return specific.subscribe(subscriber);
    };

    BENCHMARK("Dynamic observable subscribe specific subscriber")
    {
        return dynamic.subscribe(subscriber);
    };

    auto dynamic_subscriber = rpp::dynamic_subscriber([](const int&){});
    BENCHMARK("Specific observable subscribe dynamic observer")
    {
        return specific.subscribe(dynamic_subscriber);
    };

    BENCHMARK("Dynamic observable subscribe dynamic observer")
    {
        return dynamic.subscribe(dynamic_subscriber);
    };
}


TEST_CASE("Observer construction", "[benchmark]")
{
    BENCHMARK("Specific observer construction")
    {
        return MakeObserver<rpp::specific_observer>();
    };

    BENCHMARK("Dynamic observer construction")
    {
        return MakeObserver<rpp::dynamic_observer>();
    };

    BENCHMARK("Specific observer construction + as_dynamic")
    {
        return MakeObserver<rpp::specific_observer>().as_dynamic();
    };
}

TEST_CASE("OnNext", "[benchmark]")
{
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

}

TEST_CASE("Subscriber construction", "[benchmark]")
{
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
}

TEST_CASE("Observable subscribe", "[benchmark]")
{
    auto validate_observable = [](auto observable, const std::string& observable_prefix)
    {
        auto validate_with_observer = [&](const auto& observer, const std::string& observer_prefix)
        {
            BENCHMARK(observable_prefix+ " observable subscribe " + observer_prefix +" observer")
            {
                return observable.subscribe(observer);
            };
        };
        validate_with_observer(MakeObserver<rpp::specific_observer>(), "specific");
        validate_with_observer(MakeObserver<rpp::dynamic_observer>(), "dynamic");
    };

    validate_observable(MakeSpecificObservable(), "Specific");
    validate_observable(MakeDynamicObservable(), "Dynamic");
}

TEST_CASE("Observable lift", "[benchmark]")
{
    auto validate_observable = [](auto observable, const std::string& observable_prefix)
    {
        auto validate_with_observer = [&](const auto& observer, const std::string& observer_prefix)
        {
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

TEST_CASE("Operators", "[benchmark]")
{
    auto obs = rpp::observable::create<int>([](const auto& sub)
                {
                    sub.on_next(1);
                });
    auto sub = rpp::specific_subscriber{[](const int&){}};
    BENCHMARK("map construction from observable via dot + subscribe")
    {
        return obs.map([](const auto& v)
        {
            return v * 100;
        }).subscribe(sub);
    };
}

TEST_CASE("Subscription", "[benchmark]")
{
    BENCHMARK("composite_subscription create")
    {
        return rpp::composite_subscription{};
    };

    rpp::composite_subscription sub_1{};

    BENCHMARK("composite_subscription unsubscribe")
    {
        sub_1.unsubscribe();
    };
}

TEST_CASE("foundamental sources", "[benchmark]")
{
    rpp::composite_subscription sub{};

    auto err = std::make_exception_ptr(std::runtime_error{ "" });
    BENCHMARK("empty")
    {
        return rpp::source::empty<int>().subscribe(sub);
    };
    BENCHMARK("error")
    {
        return rpp::source::error<int>(err).subscribe(sub);
    };
    BENCHMARK("never")
    {
        return rpp::source::never<int>().subscribe(sub);
    };
}


TEST_CASE("just", "[benchmark]")
{
    rpp::composite_subscription sub{};

    BENCHMARK("just send int")
    {
        return rpp::source::just(1).subscribe(sub);
    };

    BENCHMARK("just send variadic")
    {
        return rpp::source::just(1,2,3,4,5,6,7,8,9,10).subscribe(sub);
    };
}

TEST_CASE("from", "[benchmark]")
{
    rpp::composite_subscription sub{};

    std::vector vec{1};
    BENCHMARK("from vector with int")
    {
        return rpp::source::from(vec).subscribe(sub);
    };
}
