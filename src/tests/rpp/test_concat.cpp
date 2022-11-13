//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/concat.hpp>
#include <rpp/subjects/publish_subject.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/never.hpp>

TEST_CASE("concat merges emissions sequentially", "[concat]")
{
    auto mock = mock_observer<int>{};

    auto subj_1 = rpp::subjects::publish_subject<int>{};
    auto subj_2 = rpp::subjects::publish_subject<int>{};

    auto validate_concat_logic = [&]()
    {
        AND_WHEN("subjects are emit values")
        {
            subj_1.get_subscriber().on_next(1);
            subj_2.get_subscriber().on_next(2);
            THEN("only values from first subject obtained")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
        AND_WHEN("first subject is completed")
        {
            subj_1.get_subscriber().on_completed();
            THEN("no values and no complete")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
            AND_WHEN("subjects are emit values")
            {
                subj_1.get_subscriber().on_next(1);
                subj_2.get_subscriber().on_next(2);
                THEN("only values from second subject obtained and no complete")
                {
                    CHECK(mock.get_received_values() == std::vector{ 2 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
            AND_WHEN("second subject is completed")
            {
                subj_2.get_subscriber().on_completed();
                THEN("subscriber completed")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
    };

    GIVEN("observable of subjects")
    {
        auto obs = rpp::observable::just(subj_1.get_observable(), subj_2.get_observable());

        WHEN("subscribe on it via concat")
        {
            obs.concat().subscribe(mock);
            validate_concat_logic();
        }
    }
    GIVEN("two subjects")
    {
        WHEN("subscribe on it via concat_with")
        {
            subj_1.get_observable().concat_with(subj_2.get_observable()).subscribe(mock);
            validate_concat_logic();
        }
    }
}

TEST_CASE("concat handles corner cases", "[concat]")
{
    auto mock = mock_observer<int>{};

    GIVEN("observable of values + error + observables")
    {
        auto obs = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                     rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic(),
                                     rpp::source::just(2).as_dynamic());
        WHEN("subscribe on it via concat")
        {
            obs.concat().subscribe(mock);
            THEN("first value obtained, then error and unsubscribe")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }

    GIVEN("observable of values + empty + observables")
    {
        auto obs = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                     rpp::source::empty<int>().as_dynamic(),
                                     rpp::source::just(2).as_dynamic());
        WHEN("subscribe on it via concat")
        {
            obs.concat().subscribe(mock);
            THEN("first value obtained, empty 'skipped', last value obtained")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("observable of values + never + observables")
    {
        auto obs = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                     rpp::source::never<int>().as_dynamic(),
                                     rpp::source::just(2).as_dynamic());
        WHEN("subscribe on it via concat")
        {
            obs.concat().subscribe(mock);
            THEN("first value obtained, then it stops on never due to no complete")
            {
                CHECK(mock.get_received_values() == std::vector{ 1});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}
