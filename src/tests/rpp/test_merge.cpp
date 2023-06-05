//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>

#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/schedulers/immediate.hpp>
#include <rpp/schedulers/new_thread.hpp>
#include <snitch/snitch_macros_check.hpp>

#include "mock_observer.hpp"

#include <stdexcept>
#include <string>

namespace snitch
{
bool append(snitch::small_string_span ss, const std::vector<int>& c) {
    std::string res{};

    for(auto v : c) {
        res += std::to_string(v) + " ";
    }
    return append(ss, res);
}
}

TEST_CASE("merge for observable of observables")
{
    auto mock = mock_observer_strategy<int>();
    SECTION("observable of observables")
    {
        auto obs= rpp::source::just(rpp::source::just(1), rpp::source::just(2));

        SECTION("subscribe on merge of observable")
        {
            obs | rpp::operators::merge() | rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values FROM underlying observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 1,2 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    SECTION("observable of observables with first never")
    {
        auto obs = rpp::source::just(rpp::source::never<int>().as_dynamic(), rpp::source::just(2).as_dynamic());

        SECTION("subscribe on merge of observable")
        {
            obs | rpp::ops::merge() | rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); //no complete due to first observable sends nothing
            }
        }
    }
    SECTION("observable of observables without complete")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
        {
            sub.on_next(rpp::source::just(1).as_dynamic());
            sub.on_next(rpp::source::just(2).as_dynamic());
        });

        SECTION("subscribe on merge of observable")
        {
            obs | rpp::ops::merge() | rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2 });
                CHECK(mock.get_on_completed_count() == 0); //no complete due to root observable is not completed
            }
        }
    }
    SECTION("observable of observables one with error")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
            {
                sub.on_next(rpp::source::just(rpp::schedulers::immediate{}, 1).as_dynamic());
                sub.on_next(rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic());
                sub.on_next(rpp::source::just(rpp::schedulers::immediate{}, 2).as_dynamic());
            });

        SECTION("subscribe on merge of observable")
        {
            obs | rpp::ops::merge() | rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0); //no complete due to error
            }
        }
    }
    SECTION("observable of observables with error")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
        {
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
            sub.on_next(rpp::source::just(1).as_dynamic());
        });

        SECTION("subscribe on merge of observable")
        {
            obs | rpp::ops::merge() | rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0); //no complete due to error
            }
        }
    }
}

TEST_CASE("merge_with")
{
    auto mock = mock_observer_strategy<int>();
    SECTION("2 observables")
    {
        auto obs_1 = rpp::source::just(1);
        auto obs_2 = rpp::source::just(2);

        SECTION("subscribe on merge of this observables")
        {
            obs_1 | rpp::ops::merge_with(obs_2) | rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values FROM both observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 1,2 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    SECTION("never observable with just observable")
    {
        auto obs_1 = rpp::source::never<int>();
        auto obs_2 = rpp::source::just(2);

        SECTION("subscribe on merge of this observables")
        {
            obs_1 | rpp::ops::merge_with(obs_2)| rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values FROM both observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); // first observable never completes
            }
        }

        SECTION("subscribe on merge of this observables in reverse oreder")
        {
            obs_2 | rpp::ops::merge_with(obs_1)| rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values FROM both observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); // first observable never completes
            }
        }
    }

    SECTION("error observable with just observable")
    {
        auto obs_1 = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        auto obs_2 = rpp::source::just(2);

        SECTION("subscribe on merge of this observables")
        {
            obs_1 | rpp::ops::merge_with(obs_2)| rpp::ops::subscribe(mock.get_observer());
            SECTION("observer obtains values FROM both observables")
            {
                CHECK(mock.get_total_on_next_count()==0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }

    }
}

TEST_CASE("merge serializes emissions")
{
    SECTION("observables from different threads")
    {
        auto s1 = rpp::source::just(rpp::schedulers::new_thread{}, 1);
        auto s2 = rpp::source::just(rpp::schedulers::new_thread{}, 2);
        SECTION("subscribe on merge of this observables")
        {
            SECTION("resulting observable emits items sequentially")
            {
                std::atomic_size_t                          counter{};
                size_t max_value = 0;
                s1 | rpp::ops::merge_with(s2) | rpp::ops::as_blocking() | rpp::ops::subscribe([&](const auto&)
                {
                    if (++counter >= 2) FAIL("++counter >= 2");
                    max_value = std::max(counter.load(), max_value);

                    std::this_thread::sleep_for(std::chrono::seconds{1});

                    max_value = std::max(counter.load(), max_value);
                    --counter;
                });
                CHECK(max_value == 1);
            }
        }
    }
}

// TEST_CASE("merge doesn't produce extra copies")
// {
//     SECTION("observable and subscriber")
//     {
//         copy_count_tracker verifier{};
//         auto          obs = rpp::source::just(verifier.get_observable()) | rpp::ops::merge() ;
//         SECTION("subscribe")
//         {
//             obs.subscribe([](copy_count_tracker){});
//             SECTION("no extra copies")
//             {
//                 // 1 copy to final lambda
//                 REQUIRE(verifier.get_copy_count() == 1);
//                 REQUIRE(verifier.get_move_count() == 0);
//             }
//         }
//     }
// }

// TEST_CASE("merge doesn't produce copies for move")
// {
//     SECTION("observable and subscriber")
//     {
//         copy_count_tracker verifier{};
//         auto          obs = rpp::source::just(verifier.get_observable_for_move()) | rpp::ops::merge() ;
//         SECTION("subscribe")
//         {
//             obs.subscribe([](copy_count_tracker) {});
//             SECTION("no extra copies")
//             {
//                 REQUIRE(verifier.get_copy_count() == 0);
//                 // 1 move to final lambda
//                 REQUIRE(verifier.get_move_count() == 1);
//             }
//         }
//     }
// }

TEST_CASE("merge handles race condition", "[merge]")
{
    SECTION("source observable in current thread pairs with error in other thread")
    {
        std::atomic_bool on_error_called{false};
        std::optional<rpp::dynamic_observer<int>> extracted_obs{};
        auto delayed_obs = rpp::source::create<int>([&](auto&& obs)
        {
            extracted_obs.emplace(std::move(obs).as_dynamic());
        });

        SECTION("subscribe on it")
        {
            SECTION("on_error can't interleave with on_next")
            {
                rpp::source::just(1, 1, 1)
                         | rpp::ops::merge_with(delayed_obs)
                         | rpp::ops::as_blocking()
                         | rpp::ops::subscribe([&](auto&&)
                                   {
                                       REQUIRE(extracted_obs.has_value());
                                       CHECK(!on_error_called);
                                       std::thread{[&]
                                       {
                                           extracted_obs->on_error(std::exception_ptr{});
                                       }}.detach();
                                       std::this_thread::sleep_for(std::chrono::seconds{1});
                                       CHECK(!on_error_called);
                                   },
                                   [&](auto) { on_error_called = true; });

                CHECK(on_error_called);
            }
        }
    }
}