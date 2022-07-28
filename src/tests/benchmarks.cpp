//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <rpp/observables.hpp>
#include <rpp/sources.hpp>
#include <rpp/observers/specific_observer.hpp>
#include <rpp/operators.hpp>
#include <rpp/subjects.hpp>

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

TEST_CASE("Observable construction")
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

TEST_CASE("Observable subscribe #2")
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

TEST_CASE("Observer construction")
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

TEST_CASE("OnNext")
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

TEST_CASE("Subscriber construction")
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

TEST_CASE("Observable subscribe")
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

TEST_CASE("Observable lift")
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

TEST_CASE("Subscription")
{
    BENCHMARK("composite_subscription create")
    {
        return rpp::composite_subscription{};
    };

    BENCHMARK_ADVANCED("composite_subscription add")(Catch::Benchmark::Chronometer meter)
    {
        rpp::composite_subscription sub{};
        rpp::composite_subscription sub_1{};
        meter.measure([&] { return sub.add(sub_1); });
    };

    BENCHMARK_ADVANCED("composite_subscription unsubscribe")(Catch::Benchmark::Chronometer meter)
    {
        rpp::composite_subscription sub{};
        meter.measure([&] { sub.unsubscribe(); });
    };
}

TEST_CASE("foundamental sources")
{
    BENCHMARK_ADVANCED("empty")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&] { return rpp::source::empty<int>().subscribe(sub); });
    };

    BENCHMARK_ADVANCED("error")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>([](int){}, [](const std::exception_ptr& err){});

        auto err = std::make_exception_ptr(std::runtime_error{""});

        meter.measure([&] { return rpp::source::error<int>(err).subscribe(sub); });
    };

    BENCHMARK_ADVANCED("never")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&] { return rpp::source::never<int>().subscribe(sub); });
    };
}

TEST_CASE("map")
{
    BENCHMARK_ADVANCED("map construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
        auto sub = rpp::specific_subscriber{[](const int&) {}};

        meter.measure([&]
        {
            return obs.map([](const auto& v)
            {
                return v * 100;
            }).subscribe(sub);
        });
    };
}

TEST_CASE("scan")
{
    BENCHMARK_ADVANCED("scan construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
        auto sub = rpp::specific_subscriber{[](const std::vector<int>&) {}};

        meter.measure([&]
        {
            return obs.scan(std::vector<int>{},
                            [](std::vector<int>&& seed, const auto& v)
                            {
                                return std::move(seed);
                            }).subscribe(sub);
        });
    };
}

TEST_CASE("distinct_until_changed")
{
    BENCHMARK_ADVANCED("distinct_until_changed construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(1);
            sub.on_next(1);
        });
        auto sub = rpp::specific_subscriber{[](const int&) {}};

        meter.measure([&]
        {
            return obs.distinct_until_changed().subscribe(sub);
        });
    };
}

TEST_CASE("with_latest_from")
{
    BENCHMARK_ADVANCED("with_latest_from construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
        auto sub = rpp::specific_subscriber{[](const std::tuple<int,int,int>&) {}};

        meter.measure([&]
        {
            return obs.with_latest_from(obs, obs).subscribe(sub);
        });
    };
}

TEST_CASE("switch_on_next")
{
    BENCHMARK_ADVANCED("switch_on_next construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        auto inner_source = rpp::source::just(1);
        using inner_source_type = decltype(inner_source);
        const auto obs = rpp::observable::create<inner_source_type>([&](const auto& sub)
        {
            sub.on_next(inner_source);
            sub.on_next(inner_source);
            sub.on_next(inner_source);
        });
        auto sub = rpp::specific_subscriber{[](const int&) {}};

        meter.measure([&]
        {
            return obs.switch_on_next().subscribe(sub);
        });
    };
}

TEST_CASE("observe_on")
{
    BENCHMARK_ADVANCED("observe_on construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(1);
            sub.on_next(1);
        });
        auto sub = rpp::specific_subscriber{[](const int&) {}};
        auto scheduler = rpp::schedulers::immediate{};
        meter.measure([&]
        {
            return obs.observe_on(scheduler).subscribe(sub);
        });
    };
}

TEST_CASE("repeat")
{
    BENCHMARK_ADVANCED("repeat construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_completed();
        });
        auto sub = rpp::specific_subscriber{[](const int&) {}};

        meter.measure([&]
        {
            return obs.repeat(10).subscribe(sub);
        });
    };
}

TEST_CASE("just")
{
    BENCHMARK_ADVANCED("just send int")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&] { return rpp::source::just(1).subscribe(sub); });
    };

    BENCHMARK_ADVANCED("just send variadic")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&] { return rpp::source::just(1, 2, 3, 4, 5, 6, 7, 8, 9, 10).subscribe(sub); });
    };
}

TEST_CASE("from")
{
    BENCHMARK_ADVANCED("from vector with int")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        std::vector vec{ 1 };
        meter.measure([&] {return rpp::source::from_iterable(vec).subscribe(sub); });
    };
}

TEST_CASE("merge")
{
    BENCHMARK_ADVANCED("merge")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&]
        {
            return rpp::source::just(rpp::source::just(1),
                                     rpp::source::just(2))
                   .merge()
                   .subscribe(sub);
        });
    };
    BENCHMARK_ADVANCED("merge_with")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&]
        {
            return rpp::source::just(1)
                   .merge_with(rpp::source::just(2))
                   .subscribe(sub);
        });
    };
}

TEST_CASE("concat")
{
    BENCHMARK_ADVANCED("concat")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&]
        {
            return rpp::source::just(rpp::source::just(1),
                                     rpp::source::just(2))
                   .concat()
                   .subscribe(sub);
        });
    };
    BENCHMARK_ADVANCED("concat_with")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::make_specific_subscriber<int>();

        meter.measure([&]
        {
            return rpp::source::just(1)
                   .concat_with(rpp::source::just(2))
                   .subscribe(sub);
        });
    };
}

TEST_CASE("publish_subject callbacks")
{
    BENCHMARK_ADVANCED("on_next")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        auto sub  = subj.get_subscriber();

        meter.measure([&]
        {
            sub.on_next(1);
        });
    };
    BENCHMARK_ADVANCED("on_error")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        auto sub  = subj.get_subscriber();
        auto err = std::make_exception_ptr(std::runtime_error{ "" });

        meter.measure([&]
        {
            sub.on_error(err);
        });
    };
    BENCHMARK_ADVANCED("on_completed")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        auto sub = subj.get_subscriber();

        meter.measure([&]
            {
                sub.on_completed();
            });
    };
}

TEST_CASE("publish_subject routines")
{
    BENCHMARK_ADVANCED("construct")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rpp::composite_subscription();

        meter.measure([&]
        {
            return rpp::subjects::publish_subject<int>{sub};
        });
    };
    BENCHMARK_ADVANCED("get_observable")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        meter.measure([&]
        {
            return subj.get_observable();
        });
    };
    BENCHMARK_ADVANCED("get_subscriber")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        meter.measure([&]
            {
                return subj.get_subscriber();
            });
    };
}
