//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/sources.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>

SCENARIO("merge for observable of observables")
{
    auto mock = mock_observer<int>();
    GIVEN("observable of observables")
    {
        auto obs= rpp::source::just(rpp::source::just(1), rpp::source::just(2));

        WHEN("subscribe on merge of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values FROM underlying observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 1,2 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("observable of observables with first never")
    {
        auto obs = rpp::source::just(rpp::source::never<int>().as_dynamic(), rpp::source::just(2).as_dynamic());

        WHEN("subscribe on merge of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); //no complete due to first observable sends nothing
            }
        }
    }
    GIVEN("observable of observables without complete")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
        {
            sub.on_next(rpp::source::just(1).as_dynamic());
            sub.on_next(rpp::source::just(2).as_dynamic());
        });

        WHEN("subscribe on merge of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2 });
                CHECK(mock.get_on_completed_count() == 0); //no complete due to root observable is not completed
            }
        }
    }
    GIVEN("observable of observables one with error")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
            {
                sub.on_next(rpp::source::just(1).as_dynamic());
                sub.on_next(rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic());
                sub.on_next(rpp::source::just(2).as_dynamic());
            });

        WHEN("subscribe on merge of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0); //no complete due to error
            }
        }
    }
    GIVEN("observable of observables with error")
    {
        auto obs = rpp::source::create<rpp::dynamic_observable<int>>([](const auto& sub)
        {
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
            sub.on_next(rpp::source::just(1).as_dynamic());
        });

        WHEN("subscribe on merge of observable")
        {
            obs.merge().subscribe(mock);
            THEN("observer obtains values from second observable even if first emits nothing")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0); //no complete due to error
            }
        }
    }
}
SCENARIO("merge_with")
{
    auto mock = mock_observer<int>();
    GIVEN("2 observables")
    {
        auto obs_1 = rpp::source::just(1);
        auto obs_2 = rpp::source::just(2);

        WHEN("subscribe on merge of this observables")
        {
            obs_1.merge_with(obs_2).subscribe(mock);
            THEN("observer obtains values FROM both observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 1,2 });
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("never observable with just observable")
    {
        auto obs_1 = rpp::source::never<int>();
        auto obs_2 = rpp::source::just(2);

        WHEN("subscribe on merge of this observables")
        {
            obs_1.merge_with(obs_2).subscribe(mock);
            THEN("observer obtains values FROM both observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); // first observable never completes
            }
        }

        WHEN("subscribe on merge of this observables in reverse oreder")
        {
            obs_2.merge_with(obs_1).subscribe(mock);
            THEN("observer obtains values FROM both observables")
            {
                CHECK(mock.get_received_values() == std::vector{ 2 });
                CHECK(mock.get_on_completed_count() == 0); // first observable never completes
            }
        }
    }

    GIVEN("error observable with just observable")
    {
        auto obs_1 = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
        auto obs_2 = rpp::source::just(2);

        WHEN("subscribe on merge of this observables")
        {
            obs_1.merge_with(obs_2).subscribe(mock);
            THEN("observer obtains values FROM both observables")
            {
                CHECK(mock.get_total_on_next_count()==0);
                CHECK(mock.get_on_error_count() == 1); 
                CHECK(mock.get_on_completed_count() == 0); 
            }
        }

    }
}

SCENARIO("merge serializes emissions")
{
    GIVEN("observables from different threads")
    {
        auto s1 = rpp::source::just(rpp::schedulers::new_thread{}, 1);
        auto s2 = rpp::source::just(rpp::schedulers::new_thread{}, 2);
        WHEN("subscribe on merge of this observables")
        {
            THEN("resulting observable emits items sequentially")
            {
                std::atomic_size_t                          counter{};
                size_t max_value = 0;
                s1.merge_with(s2).as_blocking().subscribe([&](const auto&)
                {
                    CHECK(++counter < 2);
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
