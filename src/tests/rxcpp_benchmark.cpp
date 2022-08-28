#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <rxcpp/rx.hpp>

#include <tuple>


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
    return rxcpp::observable<>::create<int>([](const auto&){});
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

TEST_CASE("Subscription")
{
    BENCHMARK("composite_subscription create")
    {
        return rxcpp::composite_subscription{};
    };

    BENCHMARK_ADVANCED("composite_subscription add")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::composite_subscription sub{};
        std::vector<rxcpp::composite_subscription> inner(meter.runs(), rxcpp::composite_subscription{});
        meter.measure([&](int i){ return sub.add(inner[i]); });
    };

    BENCHMARK_ADVANCED("composite_subscription unsubscribe")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::composite_subscription> subs(meter.runs(), rxcpp::composite_subscription{});
        meter.measure([&](int i) { subs[i].unsubscribe(); });
    };
}

TEST_CASE("foundamental sources")
{
    BENCHMARK_ADVANCED("empty")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i) { return rxcpp::observable<>::empty<int>().subscribe(subs[i]); });
    };

    BENCHMARK_ADVANCED("error")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>([](int){}, [](std::exception_ptr){}));

        auto err = std::make_exception_ptr(std::runtime_error{ "" });

        meter.measure([&](int i) { return rxcpp::observable<>::error<int>(err).subscribe(subs[i]); });
    };

    BENCHMARK_ADVANCED("never")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i) { return rxcpp::observable<>::never<int>().subscribe(subs[i]); });
    };
}

TEST_CASE("map")
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

    BENCHMARK_ADVANCED("sending of values from observable via map to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
        {
            meter.measure([&]
            {
                sub.on_next(1);
            });
        }).map([](const auto& v)
        {
            return v * 100;
        }).subscribe([](const auto&) {});
    };
}

TEST_CASE("scan")
{
    BENCHMARK_ADVANCED("scan construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
        auto sub = rxcpp::make_subscriber<std::vector<int>>([](const auto&) {});

        meter.measure([&]
        {
            return obs.scan(std::vector<int>{},
                            [](std::vector<int>&& seed, const auto&)
                            {
                                return std::move(seed);
                            }).subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via scan to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(1);
                    });
                })
                .scan(std::vector<int>{},
                      [](std::vector<int>&& seed, const auto&)
                      {
                          return std::move(seed);
                      })
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("distinct_until_changed")
{
    BENCHMARK_ADVANCED("distinct_until_changed construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(1);
            sub.on_next(1);
        });
        auto sub = rxcpp::make_subscriber<int>([](const auto&) {});

        meter.measure([&]
        {
            return obs.distinct_until_changed().subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via distinct_until_changed to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(1);
                    });
                })
                .distinct_until_changed()
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("with_latest_from")
{
    BENCHMARK_ADVANCED("with_latest_from construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
        auto sub = rxcpp::make_subscriber<std::tuple<int, int, int>>([](const auto&) {});

        meter.measure([&]
        {
            return obs.with_latest_from(obs,obs).subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via with_latest_from to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(1);
                    });
                })
                .with_latest_from(rxcpp::sources::just(1))
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("combine_latest")
{
    BENCHMARK_ADVANCED("combine_latest construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(2);
            sub.on_next(3);
        });
        auto sub = rxcpp::make_subscriber<std::tuple<int, int>>([](const auto&) {});

        meter.measure([&]
        {
            return obs.combine_latest(obs).subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via combine_latest to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
            {
                meter.measure([&]
                {
                    sub.on_next(1);
                });
            })
            .combine_latest(rxcpp::sources::just(1))
            .subscribe([](const auto&) {});
    };
}

TEST_CASE("switch_on_next")
{
    BENCHMARK_ADVANCED("switch_on_next construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        auto inner_source = rxcpp::sources::just(1);
        using inner_source_type = decltype(inner_source);

        const auto obs = rxcpp::sources::create<inner_source_type>([&](const auto& sub)
        {
            sub.on_next(inner_source);
            sub.on_next(inner_source);
            sub.on_next(inner_source);
        });
        auto sub = rxcpp::make_subscriber<int>([](const auto&) {});

        meter.measure([&]
        {
            return obs.switch_on_next().subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via switch_on_next to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        auto inner_source = rxcpp::sources::just(1);
        using inner_source_type = decltype(inner_source);

        rxcpp::sources::create<inner_source_type>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(inner_source);
                    });
                })
                .switch_on_next()
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("observe_on")
{
    BENCHMARK_ADVANCED("observe_on construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(1);
            sub.on_next(1);
        });
        auto sub = rxcpp::make_subscriber<int>([](const int&) {});
        auto scheduler = rxcpp::identity_immediate();
        meter.measure([&]
        {
            return obs.observe_on(scheduler).subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via observe_on to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(1);
                    });
                })
                .observe_on(rxcpp::identity_immediate())
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("repeat")
{
    BENCHMARK_ADVANCED("repeat construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_completed();
        });

        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return obs.repeat(10).subscribe(subs[i]);
        });
    };
}

TEST_CASE("just")
{
    BENCHMARK_ADVANCED("just send int")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i) { return rxcpp::sources::just(1).subscribe(subs[i]); });
    };

    BENCHMARK_ADVANCED("just send variadic")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i) { return rxcpp::sources::from(1, 2, 3, 4, 5, 6, 7, 8, 9, 10).subscribe(subs[i]); });
    };
}

TEST_CASE("from")
{
    BENCHMARK_ADVANCED("from vector with int")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        std::vector vec{ 1 };
        meter.measure([&](int i) {return rxcpp::sources::iterate(vec).subscribe(subs[i]); });
    };
}

TEST_CASE("merge")
{
    BENCHMARK_ADVANCED("merge")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return rxcpp::sources::from(rxcpp::sources::just(1),
                                        rxcpp::sources::just(2))
                   .merge()
                   .subscribe(subs[i]);
        });
    };
    BENCHMARK_ADVANCED("merge_with")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return rxcpp::sources::just(1)
                   .merge(rxcpp::sources::just(2))
                   .subscribe(subs[i]);
        });
    };
}

TEST_CASE("concat")
{
    BENCHMARK_ADVANCED("concat")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return rxcpp::sources::from(rxcpp::sources::just(1),
                                        rxcpp::sources::just(2))
                   .concat()
                   .subscribe(subs[i]);
        });
    };
    BENCHMARK_ADVANCED("concat_with")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return rxcpp::sources::just(1)
                   .concat(rxcpp::sources::just(2))
                   .subscribe(subs[i]);
        });
    };
}

TEST_CASE("buffer")
{
    BENCHMARK_ADVANCED("buffer")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<std::vector<int>>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<std::vector<int>>());

        meter.measure([&](int i)
            {
                return rxcpp::observable<>::from(1,2,3,4,5)
                    .buffer(2)
                    .subscribe(subs[i]);
            });
    };

    BENCHMARK_ADVANCED("sending of values from observable via buffer to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(1);
                    });
                })
                .buffer(1)
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("window")
{
    BENCHMARK_ADVANCED("window")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<rxcpp::operators::detail::window<int>::value_type>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<rxcpp::operators::detail::window<int>::value_type>());

        meter.measure([&](int i)
        {
            return rxcpp::observable<>::from(1, 2, 3, 4, 5)
                   .window(2)
                   .subscribe(subs[i]);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via window to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
                {
                    meter.measure([&]
                    {
                        sub.on_next(1);
                    });
                })
                .window(1)
                .subscribe([](const auto&) {});
    };
}

TEST_CASE("take")
{
    BENCHMARK_ADVANCED("take construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
            {
                sub.on_next(1);
            });

        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
            {
                return obs.take(1).subscribe(subs[i]);
            });
    };

    BENCHMARK_ADVANCED("sending of values from observable via take to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
            {
                meter.measure([&]
                    {
                        sub.on_next(1);
                    });
            })
            .take(meter.runs() - 1)
            .subscribe([](const auto&) {});
    };
}

TEST_CASE("first")
{
    BENCHMARK_ADVANCED("first construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
            {
                sub.on_next(1);
            });

        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
            {
                return obs.first().subscribe(subs[i]);
            });
    };

    BENCHMARK_ADVANCED("sending of values from observable via first to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
            {
                meter.measure([&]
                    {
                        sub.on_next(1);
                    });
            })
            .first()
            .subscribe([](const auto&) {});
    };
}

TEST_CASE("skip")
{
    BENCHMARK_ADVANCED("skip construction from observable via dot + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        const auto obs = rxcpp::sources::create<int>([](const auto& sub)
        {
            sub.on_next(1);
        });
        auto sub = rxcpp::make_subscriber<int>([](const int&) {});

        meter.measure([&]
        {
            return obs.skip(1).subscribe(sub);
        });
    };

    BENCHMARK_ADVANCED("sending of values from observable via skip to subscriber")(Catch::Benchmark::Chronometer meter)
    {
        rxcpp::sources::create<int>([&](const auto& sub)
        {
            meter.measure([&]
            {
                sub.on_next(1);
            });
        })
        .skip(meter.runs()-1)
        .subscribe([](const auto&) {});
    };
}

TEST_CASE("chains creation test")
{
    BENCHMARK_ADVANCED("long non-state chain creation + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return rxcpp::observable<>::never<int>()
                   .map([](int       ) { return 0; })
                   .filter([](int    ) { return true; })
                   .take_while([](int) { return true; })
                   .subscribe(subs[i]);
        });
    };

    BENCHMARK_ADVANCED("long stateful chain creation + subscribe")(Catch::Benchmark::Chronometer meter)
    {
        std::vector<rxcpp::subscriber<int>> subs{};
        for (int i = 0; i < meter.runs(); ++i)
            subs.push_back(rxcpp::make_subscriber<int>());

        meter.measure([&](int i)
        {
            return rxcpp::observable<>::never<int>().take(1)
                                                    .skip(1)
                                                    .distinct_until_changed()
                                                    .scan(int{}, [](int, int) { return 0; })
                                                    .subscribe(subs[i]);
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

TEST_CASE("immediate scheduler")
{
    BENCHMARK_ADVANCED("no any re-schedule")(Catch::Benchmark::Chronometer meter)
    {
        auto scheduler = rxcpp::schedulers::make_immediate();
        auto worker    = scheduler.create_worker();
        auto time      = scheduler.now();

        auto work = [](const rxcpp::schedulers::schedulable&) { };
        meter.measure([&]
        {
            worker.schedule(time, work);
        });
    };

    BENCHMARK_ADVANCED("re-schedule 10 times")(Catch::Benchmark::Chronometer meter)
    {
        auto scheduler = rxcpp::schedulers::make_immediate();
        auto worker    = scheduler.create_worker();
        auto time      = scheduler.now();
        int  count     = 0;

        auto work = [&count](const rxcpp::schedulers::schedulable& self)
        {
            if (count++ >= 10)
                return;

            self();
        };
        meter.measure([&]
        {
            count = 0;

            worker.schedule(time, work);
        });
    };
}

TEST_CASE("trampoline scheduler")
{
    BENCHMARK_ADVANCED("no any re-schedule")(Catch::Benchmark::Chronometer meter)
    {
        auto scheduler = rxcpp::schedulers::make_current_thread();
        auto worker    = scheduler.create_worker();
        auto time      = scheduler.now();

        auto work = [](const rxcpp::schedulers::schedulable&) {};
        meter.measure([&]
        {
            worker.schedule(time, work);
        });
    };

    BENCHMARK_ADVANCED("re-schedule 10 times")(Catch::Benchmark::Chronometer meter)
    {
        auto scheduler = rxcpp::schedulers::make_current_thread();
        auto worker    = scheduler.create_worker();
        auto time      = scheduler.now();
        int  count     = 0;

        auto work = [&count](const rxcpp::schedulers::schedulable& self)
        {
            if (count++ >= 10)
                return;

            self();
        };
        meter.measure([&]
        {
            count = 0;

            worker.schedule(time, work);
        });
    };

    BENCHMARK_ADVANCED("recursively schedule 10 times")(Catch::Benchmark::Chronometer meter)
    {
        auto scheduler = rxcpp::schedulers::make_current_thread();
        auto worker    = scheduler.create_worker();
        auto time      = scheduler.now();

        auto work = [&](const rxcpp::schedulers::schedulable&)
        {
            for (size_t i = 0; i < 10; ++i)
                worker.schedule(time, [](const rxcpp::schedulers::schedulable&) { });
        };
        meter.measure([&]
        {
            worker.schedule(time, work);
        });
    };
}

