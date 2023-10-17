#include <nanobench.h>

#include <rpp/rpp.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <span>
#include <string_view>
#include <tuple>
#ifdef RPP_BUILD_RXCPP
    #include <rxcpp/rx.hpp>
#endif

#define BENCHMARK(NAME)                     \
    bench.context("benchmark_title", NAME); \
    if (!benchmark.has_value() || std::string_view{NAME}.find(benchmark.value()) != std::string_view::npos)
#define SECTION(NAME)                      \
    bench.context("benchmark_name", NAME); \
    if (!section.has_value() || std::string_view{NAME}.find(section.value()) != std::string_view::npos)
#define TEST_RPP(...) \
    if (!disable_rpp) bench.context("source", "rpp").run(__VA_ARGS__)
#ifdef RPP_BUILD_RXCPP
    #define TEST_RXCPP(...) \
        if (!disable_rxcpp) bench.context("source", "rxcpp").run(__VA_ARGS__)
#else
    #define TEST_RXCPP(...)
#endif

char const * json() noexcept
{
    return R"DELIM([
{{#result}}        {
            "title": "{{context(benchmark_title)}}",
            "name": "{{context(benchmark_name)}}",
            "source" : "{{context(source)}}",
            "median(elapsed)": {{median(elapsed)}},
            "medianAbsolutePercentError(elapsed)": {{medianAbsolutePercentError(elapsed)}}
        }{{^-last}},{{/-last}}
{{/result}}
])DELIM";
}

std::optional<std::string_view> find_argument(std::string_view target_argument, std::span<char*> args)
{
    for (const auto raw_argument : args)
    {
        const auto argument = std::string_view{raw_argument};
        if (argument.starts_with(target_argument))
        {
            return argument.substr(target_argument.size());
        }
    }
    return std::nullopt;
}

namespace rpp
{
    template<typename ...Ts>
    auto immediate_just(Ts&&...vals)
    {
        return rpp::source::just(rpp::schedulers::immediate{}, std::forward<Ts>(vals)...);
    }
}

namespace rxcpp
{
    template<typename ...Ts>
    auto immediate_just(Ts&&...vals)
    {
        return rxcpp::observable<>::from(rxcpp::identity_immediate(), std::forward<Ts>(vals)...);
    }
}

int main(int argc, char* argv[]) // NOLINT(bugprone-exception-escape)
{
    auto       bench         = ankerl::nanobench::Bench{}.output(nullptr).warmup(3);
    const auto args          = std::span{argv, static_cast<size_t>(argc)};
    const auto benchmark     = find_argument("--benchmark=", args);
    const auto section       = find_argument("--section=", args);
    const auto disable_rxcpp = find_argument("--disable_rxcpp", args).has_value();
    const auto disable_rpp   = find_argument("--disable_rpp", args).has_value();
    const auto dump          = find_argument("--dump=", args);

    BENCHMARK("General")
    {
        SECTION("Subscribe empty callbacks to empty observable")
        {
            TEST_RPP([&]() {
                rpp::source::create<int>([&](auto&& observer) {
                    ankerl::nanobench::doNotOptimizeAway(observer);
                })
                    .subscribe([](int) {});
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::create<int>([&](auto&& observer) {
                    ankerl::nanobench::doNotOptimizeAway(observer);
                })
                    .subscribe([](int) {});
            });
        }

        SECTION("Subscribe empty callbacks to empty observable via pipe operator")
        {
            TEST_RPP([&]() {
                rpp::source::create<int>([&](auto&& observer) {
                    ankerl::nanobench::doNotOptimizeAway(observer);
                })
                    | rpp::operators::subscribe([](int) {});
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::create<int>([&](auto&& observer) {
                    ankerl::nanobench::doNotOptimizeAway(observer);
                })
                    | rxcpp::operators::subscribe<int>([](int) {});
            });
        }
    };

    BENCHMARK("Sources")
    {
        SECTION("from array of 1 - create + subscribe + immediate")
        {
            std::array<int, 1> vals{123};
            TEST_RPP([&]() {
                rpp::source::from_iterable(vals, rpp::schedulers::immediate{}).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::iterate(vals, rxcpp::identity_immediate()).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("from array of 1 - create + subscribe + current_thread")
        {
            std::array<int, 1> vals{123};
            TEST_RPP([&]() {
                rpp::source::from_iterable(vals, rpp::schedulers::current_thread{}).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::iterate(vals, rxcpp::identity_current_thread()).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("concat_as_source of just(1 immediate) create + subscribe")
        {
            TEST_RPP([&]() {
                rpp::source::concat(rpp::source::just(rpp::schedulers::immediate{}, 1)).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::just(rxcpp::observable<>::just(1, rxcpp::identity_immediate()), rxcpp::identity_immediate()) | rxcpp::operators::concat() | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("defer from array of 1 - defer + create + subscribe + immediate")
        {
            TEST_RPP([&]() {
                rpp::source::defer([] { return rpp::source::from_iterable(std::array{123}, rpp::schedulers::immediate{}); }).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::defer([] { return rxcpp::observable<>::iterate(std::array{123}, rxcpp::identity_immediate()); }).subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("interval - interval + take(3) + subscribe + immediate")
        {
            TEST_RPP([&]() {
                rpp::source::interval(std::chrono::nanoseconds(0), rpp::schedulers::immediate{}) | rpp::operators::take(3) | rpp::operators::subscribe([](size_t v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::interval(std::chrono::nanoseconds(0), rxcpp::identity_immediate()).take(3).subscribe([](size_t v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("interval - interval + take(3) + subscribe + current_thread")
        {
            TEST_RPP([&]() {
                rpp::source::interval(std::chrono::nanoseconds(0), rpp::schedulers::current_thread{}) | rpp::operators::take(3) | rpp::operators::subscribe([](size_t v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::observable<>::interval(std::chrono::nanoseconds(0), rxcpp::identity_current_thread()).take(3).subscribe([](size_t v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
    };

    BENCHMARK("Schedulers")
    {
        SECTION("immediate scheduler create worker + schedule")
        {
            TEST_RPP([&]() {
                rpp::schedulers::immediate::create_worker().schedule([](const auto& v) { ankerl::nanobench::doNotOptimizeAway(v); return rpp::schedulers::optional_duration{}; }, rpp::make_lambda_observer([](int) {}));
            });
            TEST_RXCPP([&]() {
                rxcpp::identity_immediate().create_coordinator().get_worker().schedule([](const auto& v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("current_thread scheduler create worker + schedule")
        {
            TEST_RPP([&]() {
                rpp::schedulers::current_thread::create_worker().schedule([](const auto& v) { ankerl::nanobench::doNotOptimizeAway(v); return rpp::schedulers::optional_duration{}; }, rpp::make_lambda_observer([](int) {}));
            });
            TEST_RXCPP([&]() {
                rxcpp::identity_current_thread().create_coordinator().get_worker().schedule([](const auto& v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("current_thread scheduler create worker + schedule + recursive schedule")
        {
            TEST_RPP(
                [&]() {
                    const auto worker = rpp::schedulers::current_thread::create_worker();
                    worker.schedule(
                        [&worker](auto&& v) {
                            worker.schedule(
                                [](const auto& v) {
                                    ankerl::nanobench::doNotOptimizeAway(v);
                                    return rpp::schedulers::optional_duration{};
                                },
                                std::move(v));
                            return rpp::schedulers::optional_duration{};
                        },
                        rpp::make_lambda_observer([](int) {}));
                });
            TEST_RXCPP(
                [&]() {
                    const auto worker = rxcpp::identity_current_thread()
                                            .create_coordinator()
                                            .get_worker();

                    worker.schedule([&worker](const auto&) {
                        worker.schedule([](const auto& v) { ankerl::nanobench::doNotOptimizeAway(v); });
                    });
                });
        }
    }

    BENCHMARK("Combining Operators")
    {
        SECTION("immediate_just(immediate_just(1), immediate_just(1)) + merge() + subscribe")
        {
            TEST_RPP([&]() {
                auto inner_source = rpp::immediate_just(1);

                rpp::immediate_just(inner_source, inner_source)
                    | rpp::operators::merge()
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                auto inner_source = rxcpp::immediate_just(1);

                rxcpp::immediate_just(inner_source, inner_source)
                    | rxcpp::operators::merge()
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just(1) + merge_with(immediate_just(2)) + subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::merge_with(rpp::immediate_just(2))
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::merge(rxcpp::immediate_just(2))
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just(1) + with_latest_from(immediate_just(2)) + subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::with_latest_from(rpp::immediate_just(2))
                    | rpp::operators::subscribe([](const std::tuple<int, int>& v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            // doesn't work due to tuple issues in rxcpp =C
            // TEST_RXCPP([&]()
            // {
            //     rxcpp::observable<>::create<int>([](const auto& obs){obs.on_next(1);})
            //     | rxcpp::operators::with_latest_from(rpp::utils::pack_to_tuple{}, rxcpp::observable<>::create<int>([](const auto& obs){obs.on_next(2);}))
            //     | rxcpp::operators::subscribe<std::tuple<int,int>>([](const std::tuple<int,int>& v){ ankerl::nanobench::doNotOptimizeAway(v); });
            // });
        }
    }

    BENCHMARK("Conditional Operators")
    {
        SECTION("immediate_just+take_while(false)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::take_while([](int) { return false; })
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::take_while([](int) { return false; })
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just+take_while(true)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::take_while([](int) { return true; })
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::take_while([](int) { return true; })
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
    };

    BENCHMARK("Transforming Operators")
    {
        SECTION("immediate_just+map(v*2)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::map([](int v) { return v * 2; })
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::map([](int v) { return v * 2; })
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just+scan(10, std::plus)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::scan(10, std::plus<int>{})
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::scan(10, std::plus<int>{})
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just+flat_map(immediate_just(v*2))+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::flat_map([](int v) { return rpp::immediate_just(v * 2); })
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::flat_map([](int v) { return rxcpp::immediate_just(v * 2); })
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just+buffer(2)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::buffer(2)
                    | rpp::operators::subscribe([](const std::vector<int>& v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::buffer(2)
                    | rxcpp::operators::subscribe<std::vector<int>>([](const std::vector<int>& v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
        SECTION("immediate_just+window(2)+subscribe + subscsribe inner")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::window(2)
                    | rpp::operators::subscribe([](const auto& v) { v.subscribe([](int vv){ankerl::nanobench::doNotOptimizeAway(vv);}); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::window(2)
                    | rxcpp::operators::subscribe<rxcpp::observable<int>>([](const rxcpp::observable<int>& v) { v.subscribe([](int vv){ankerl::nanobench::doNotOptimizeAway(vv);}); });
            });
        }
    };

    BENCHMARK("Filtering Operators")
    {
        SECTION("immediate_just+take(1)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::take(1)
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::take(1)
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just+filter(true)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::filter([](int) { return true; })
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::filter([](int) { return true; })
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just(1,2)+first()+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1, 2)
                    | rpp::operators::first()
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1, 2)
                    | rxcpp::operators::first()
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just(1,2)+last()+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1, 2)
                    | rpp::operators::last()
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1, 2)
                    | rxcpp::operators::last()
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just(1,2)+skip(1)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1, 2)
                    | rpp::operators::skip(1)
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1, 2)
                    | rxcpp::operators::skip(1)
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }

        SECTION("immediate_just(1,1,2)+distinct_until_changed()+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1, 1, 2)
                    | rpp::operators::distinct_until_changed()
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1, 1, 2)
                    | rxcpp::operators::distinct_until_changed()
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
    };

    BENCHMARK("Utility Operators")
    {
        SECTION("immediate_just(1)+subscribe_on(immediate)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::subscribe_on(rpp::schedulers::immediate{})
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::subscribe_on(rxcpp::identity_immediate())
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
    }

    BENCHMARK("Aggregating Operators")
    {
        SECTION("immediate_just+reduce(10, std::plus)+subscribe")
        {
            TEST_RPP([&]() {
                rpp::immediate_just(1)
                    | rpp::operators::reduce(10, std::plus<int>{})
                    | rpp::operators::subscribe([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });

            TEST_RXCPP([&]() {
                rxcpp::immediate_just(1)
                    | rxcpp::operators::reduce(10, std::plus<int>{})
                    | rxcpp::operators::subscribe<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
    }

    BENCHMARK("Subjects")
    {
        SECTION("publish_subject with 1 observer - on_next")
        {
            {
                rpp::subjects::publish_subject<int> rpp_subj{};
                rpp_subj.get_observable().subscribe(rpp::make_lambda_observer([](int v) { ankerl::nanobench::doNotOptimizeAway(v); }));
                TEST_RPP([&]() {
                    rpp_subj.get_observer().on_next(1);
                });
            }
            {
                rxcpp::subjects::subject<int> rxcpp_subj{};
                rxcpp_subj.get_observable().subscribe(rxcpp::make_subscriber<int>([](int v) { ankerl::nanobench::doNotOptimizeAway(v); }, [] {}));
                TEST_RXCPP([&]() {
                    rxcpp_subj.get_subscriber().on_next(1);
                });
            }
        }
    }

    BENCHMARK("Scenarios")
    {
        SECTION("basic sample")
        {
            TEST_RPP([&]() {
                rpp::source::just('1', 'a', 'W', '2', '0', 'f', 'q')
                    | rpp::operators::repeat()
                    | rpp::operators::take_while([](char v) { return v != '0'; })
                    | rpp::operators::filter([](char v) -> bool { return !std::isdigit(v); })
                    | rpp::operators::map([](char v) -> char { return std::toupper(v); })
                    | rpp::operators::subscribe([](char v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
            TEST_RXCPP([&]() {
                rxcpp::observable<>::from('1', 'a', 'W', '2', '0', 'f', 'q')
                    | rxcpp::operators::repeat()
                    | rxcpp::operators::take_while([](char v) { return v != '0'; })
                    | rxcpp::operators::filter([](char v) -> bool { return !std::isdigit(v); })
                    | rxcpp::operators::map([](char v) -> char { return std::toupper(v); })
                    | rxcpp::operators::subscribe<char>([](char v) { ankerl::nanobench::doNotOptimizeAway(v); });
            });
        }
    }

    if (dump.has_value())
    {
        std::ofstream of{std::string{dump.value()}};
        bench.render(json(), of);
        of.close();
    }

    bench.render(json(), std::cout);
}
