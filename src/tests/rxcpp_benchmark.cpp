#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <rxcpp/rx.hpp>


auto MakeSpecificObserver()
{
    return rxcpp::make_observer<int>( [](int) {});
}

auto MakeDynamicObserver()
{
    return rxcpp::make_observer_dynamic<int>([](int){});
}

auto MakeSpecificObservable()
{
    return rxcpp::observable<>::create<int>([](const auto& sub){});
}

auto MakeDynamicObservable()
{
    return MakeSpecificObservable().as_dynamic();
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

    auto subscriber = rxcpp::make_subscriber<int>([](const int&) {});
    BENCHMARK("Specific observable subscribe specific subscriber")
    {
        return specific.subscribe(subscriber);
    };

    BENCHMARK("Dynamic observable subscribe specific subscriber")
    {
        return dynamic.subscribe(subscriber);
    };

    auto dynamic_subscriber = subscriber.as_dynamic();
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
        return MakeSpecificObserver();
    };

    BENCHMARK("Dynamic observer construction")
    {
        return MakeDynamicObserver();
    };

    BENCHMARK("Specific observer construction + as_dynamic")
    {
        return MakeSpecificObserver().as_dynamic();
    };
}

TEST_CASE("OnNext")
{
    auto specific_observer = MakeSpecificObserver();
    auto dynamic_observer = MakeDynamicObserver();

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
        return rxcpp::make_subscriber<int>([](const int&) {});
    };

    auto sub = rxcpp::make_subscriber<int>([](const int&) {});

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
            BENCHMARK(observable_prefix + " observable subscribe " + observer_prefix + " observer")
            {
                return observable.subscribe(observer);
            };
        };
        validate_with_observer(MakeSpecificObserver(), "specific");
        validate_with_observer(MakeDynamicObserver(), "dynamic");
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
            BENCHMARK(observable_prefix + " observable lift " + observer_prefix + " observer")
            {
                auto res_observable = observable.template lift<int>([](const auto& v) { return v; });
                return res_observable.subscribe(observer);
            };
        };
        validate_with_observer(MakeSpecificObserver(), "specific");
        validate_with_observer(MakeDynamicObserver(), "dynamic");
    };

    validate_observable(MakeSpecificObservable(), "Specific");
    validate_observable(MakeDynamicObservable(), "Dynamic");
}

TEST_CASE("Operators")
{
    BENCHMARK_ADVANCED("map construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
            {
                sub.on_next(1);
            });
        auto sub = rxcpp::make_subscriber<int>([](const int&) {});

        meter.measure([&]
            {
                return obs.map([](const auto& v)
                    {
                        return v * 100;
                    }).subscribe(sub);
            });
    };
}

TEST_CASE("Subscription")
{
    BENCHMARK("composite_subscription create")
    {
        return rxcpp::composite_subscription{};
    };

    BENCHMARK_ADVANCED("composite_subscription add")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::composite_subscription sub{};
        rxcpp::composite_subscription sub_1{};
        meter.measure([&] { return sub.add(sub_1); });
    };

    BENCHMARK_ADVANCED("composite_subscription unsubscribe")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::composite_subscription sub{};
        meter.measure([&] { sub.unsubscribe(); });
    };
}

TEST_CASE("foundamental sources")
{
    BENCHMARK_ADVANCED("empty")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        meter.measure([&] { return rxcpp::sources::empty<int>().subscribe(sub); });
    };

    BENCHMARK_ADVANCED("error")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>([](int) {}, [](const std::exception_ptr& err) {});

        auto err = std::make_exception_ptr(std::runtime_error{ "" });

        meter.measure([&] { return rxcpp::sources::error<int>(err).subscribe(sub); });
    };

    BENCHMARK_ADVANCED("never")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        meter.measure([&] { return rxcpp::sources::never<int>().subscribe(sub); });
    };
}


TEST_CASE("just")
{
    BENCHMARK_ADVANCED("just send int")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        meter.measure([&] { return rxcpp::sources::just(1).subscribe(sub); });
    };

    BENCHMARK_ADVANCED("just send variadic")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        meter.measure([&] { return rxcpp::sources::from(1, 2, 3, 4, 5, 6, 7, 8, 9, 10).subscribe(sub); });
    };
}

TEST_CASE("from")
{
    BENCHMARK_ADVANCED("from vector with int")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        std::vector vec{ 1 };
        meter.measure([&] {return rxcpp::sources::iterate(vec).subscribe(sub); });
    };
}

TEST_CASE("merge")
{
    BENCHMARK_ADVANCED("merge")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        meter.measure([&]
            {
                return rxcpp::sources::from(rxcpp::sources::just(1),
                    rxcpp::sources::just(2))
                    .merge()
                    .subscribe(sub);
            });
    };
    BENCHMARK_ADVANCED("merge_with")(Catch::Benchmark::Chronometer meter)
    {
        auto sub = rxcpp::make_subscriber<int>();

        meter.measure([&]
            {
                return rxcpp::sources::just(1)
                    .merge(rxcpp::sources::just(2))
                    .subscribe(sub);
            });
    };
}

TEST_CASE("publish_subject callbacks")
{
    BENCHMARK_ADVANCED("on_next")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rxcpp::subjects::subject<int>{};
        auto sub  = subj.get_subscriber();

        meter.measure([&]
        {
            sub.on_next(1);
        });
    };
    BENCHMARK_ADVANCED("on_error")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rxcpp::subjects::subject<int>{};
        auto sub  = subj.get_subscriber();
        auto err  = std::make_exception_ptr(std::runtime_error{""});

        meter.measure([&]
        {
            sub.on_error(err);
        });
    };
    BENCHMARK_ADVANCED("on_completed")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rxcpp::subjects::subject<int>{};
        auto sub  = subj.get_subscriber();

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
        auto sub = rxcpp::composite_subscription();

        meter.measure([&]
            {
                return rxcpp::subjects::subject<int>{sub};
            });
    };
    BENCHMARK_ADVANCED("get_observable")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rxcpp::subjects::subject<int>{};
        meter.measure([&]
            {
                return subj.get_observable();
            });
    };
    BENCHMARK_ADVANCED("get_subscriber")(Catch::Benchmark::Chronometer meter)
    {
        auto subj = rxcpp::subjects::subject<int>{};
        meter.measure([&]
            {
                return subj.get_subscriber();
            });
    };
}