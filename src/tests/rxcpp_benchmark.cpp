#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <rxcpp/rx.hpp>

#include <array>

auto MakeSpecificObserver()
{
    std::array<int, 1> v{};

    return rxcpp::make_observer<int>( [v](int)
    {
        [[maybe_unused]] const auto& t = v;
    });
}

auto MakeDynamicObserver()
{
    std::array<int, 1> v{};

    return rxcpp::make_observer_dynamic<int>([v](int)
        {
            [[maybe_unused]] const auto& t = v;
        });
}

auto MakeSpecificObservable()
{
    std::array<int, 1> v{};

    return rxcpp::observable<>::create<int>([v](const auto& sub)
    {
        sub.on_next(static_cast<int>(v.size()));
    });
}

auto MakeDynamicObservable()
{
    std::array<int, 1> v{};

    return MakeSpecificObservable().as_dynamic();
}

TEST_CASE("Observable construction", "[benchmark]")
{
    BENCHMARK(std::string{"RxCpp "}+"Specific observable construction")
    {
        return MakeSpecificObservable();
    };

    BENCHMARK(std::string{"RxCpp "}+"Specific observable construction + as_dynamic")
    {
        return MakeSpecificObservable().as_dynamic();
    };

}

TEST_CASE("Observable subscribe #2", "[benchmark]")
{
    rxcpp::composite_subscription subscription{};
    auto specific = MakeSpecificObservable();
    BENCHMARK(std::string{"RxCpp "}+"Specific observable subscribe lambda")
    {
        return specific.subscribe(subscription, [](const auto&) {});
    };

    auto dynamic = MakeDynamicObservable();
    BENCHMARK(std::string{"RxCpp "}+"Dynamic observable subscribe lambda")
    {
        return dynamic.subscribe(subscription, [](const auto&) {});
    };

    auto subscriber = rxcpp::make_subscriber<int>([](const int&) {} );
    BENCHMARK(std::string{"RxCpp "}+"Specific observable subscribe specific subscriber")
    {
        return specific.subscribe(subscriber);
    };

    BENCHMARK(std::string{"RxCpp "}+"Dynamic observable subscribe specific subscriber")
    {
        return dynamic.subscribe(subscriber);
    };

    auto dynamic_subscriber = subscriber.as_dynamic();
    BENCHMARK(std::string{"RxCpp "}+"Specific observable subscribe dynamic observer")
    {
        return specific.subscribe(dynamic_subscriber);
    };

    BENCHMARK(std::string{"RxCpp "}+"Dynamic observable subscribe dynamic observer")
    {
        return dynamic.subscribe(dynamic_subscriber);
    };
}

TEST_CASE("Observer construction", "[benchmark]")
{
    BENCHMARK(std::string{"RxCpp "}+"Specific observer construction")
    {
        return MakeSpecificObserver();
    };

    BENCHMARK(std::string{"RxCpp "}+"Dynamic observer construction")
    {
        return MakeDynamicObserver();
    };

    BENCHMARK(std::string{"RxCpp "}+"Specific observer construction + as_dynamic")
    {
        return MakeSpecificObserver().as_dynamic();
    };
}

TEST_CASE("OnNext", "[benchmark]")
{
    auto specific_observer = MakeSpecificObserver();
    auto dynamic_observer = MakeDynamicObserver();

    BENCHMARK(std::string{"RxCpp "}+"Specific observer OnNext", i)
    {
        specific_observer.on_next(i);
        return i;
    };

    BENCHMARK(std::string{"RxCpp "}+"Dynamic observer OnNext", i)
    {
        dynamic_observer.on_next(i);
        return i;
    };

}

TEST_CASE("Subscriber construction", "[benchmark]")
{
    BENCHMARK(std::string{"RxCpp "}+"Make subsriber")
    {
        return rxcpp::make_subscriber<int>([](const int&) {});
    };

    auto sub = rxcpp::make_subscriber<int>([](const int&) {});

    BENCHMARK(std::string{"RxCpp "}+"Make copy of subscriber")
    {
        auto second = sub;
        return second;
    };

    BENCHMARK(std::string{"RxCpp "}+"Transform subsriber to dynamic")
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
            BENCHMARK(std::string{"RxCpp "}+observable_prefix + " observable subscribe " + observer_prefix + " observer")
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

TEST_CASE("Observable lift", "[benchmark]")
{
    auto validate_observable = [](auto observable, const std::string& observable_prefix)
    {
        auto validate_with_observer = [&](const auto& observer, const std::string& observer_prefix)
        {
            BENCHMARK(std::string{"RxCpp "}+observable_prefix + " observable lift " + observer_prefix + " observer")
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

TEST_CASE("Operators", "[benchmark]")
{
    auto obs = rxcpp::observable<>::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
    auto sub = rxcpp::make_subscriber<int>([](const int&) {});
    BENCHMARK(std::string{"RxCpp "}+"map construction from observable + subscribe")
    {
        return (obs | rxcpp::operators::map([](const auto& v)
            {
                return v * 100;
            })).subscribe(sub);
    };
}