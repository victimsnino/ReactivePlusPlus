//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.h"

#include <catch2/catch_test_macros.hpp>
#include <rpp/subjects/publish_subject.hpp>

SCENARIO("publish subject multicasts values")
{
    auto mock_1 = mock_observer<int>{};
    auto mock_2 = mock_observer<int>{};
    GIVEN("publish subject")
    {
        auto sub = rpp::subjects::publish_subject<int>{};
        WHEN("subscribe multiple observers")
        {
            auto sub_1 = sub.get_observable().subscribe(mock_1);
            auto sub_2 = sub.get_observable().subscribe(mock_2);

            AND_WHEN("emit value")
            {
                sub.get_subscriber().on_next(1);
                THEN("observers obtain value")
                {
                    auto validate = [](auto mock)
                    {
                        CHECK(mock.get_received_values() == std::vector{ 1 });
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    };
                    validate(mock_1);
                    validate(mock_2);
                }
            }
            AND_WHEN("emit error")
            {
                sub.get_subscriber().on_error(std::make_exception_ptr(std::runtime_error{""}));
                THEN("observers obtain error")
                {
                    auto validate = [](auto mock)
                    {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 1);
                        CHECK(mock.get_on_completed_count() == 0);
                    };
                    validate(mock_1);
                    validate(mock_2);
                }
            }
            AND_WHEN("emit on_completed")
            {
                sub.get_subscriber().on_completed();
                THEN("observers obtain on_completed")
                {
                    auto validate = [](auto mock)
                    {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                    };
                    validate(mock_1);
                    validate(mock_2);
                }
            }
            AND_WHEN("emit unsubscribe")
            {
                sub.get_subscriber().unsubscribe();
                THEN("observers obtain on_completed")
                {
                    auto validate = [](auto mock)
                    {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    };
                    validate(mock_1);
                    validate(mock_2);
                    CHECK(sub_1.is_subscribed() == false);
                    CHECK(sub_2.is_subscribed() == false);
                }
            }
            AND_WHEN("emit multiple values")
            {
                THEN("each sbuscriber obtain first value, then seconds and etc")
                {
                    sub.get_subscriber().on_next(1);
                    auto check_1 = [](auto mock) { CHECK(mock.get_received_values() == std::vector{ 1 }); };
                    check_1(mock_1);
                    check_1(mock_2);

                    sub.get_subscriber().on_next(2);
                    auto check_2 = [](auto mock) { CHECK(mock.get_received_values() == std::vector{ 1,2 }); };
                    check_2(mock_1);
                    check_2(mock_2);
                }
            }
            AND_WHEN("first subscriber unsubscribes and then emit value")
            {
                sub_1.unsubscribe();
                sub.get_subscriber().on_next(1);
                THEN("observers obtain value")
                {
                    CHECK(mock_1.get_total_on_next_count() == 0);
                    CHECK(mock_1.get_on_error_count() == 0);
                    CHECK(mock_1.get_on_completed_count() == 0);

                    CHECK(mock_2.get_received_values() == std::vector{ 1 });
                    CHECK(mock_2.get_total_on_next_count() == 1);
                    CHECK(mock_2.get_on_error_count() == 0);
                    CHECK(mock_2.get_on_completed_count() == 0);
                }
            }
        }
    }
}

SCENARIO("publish subject caches error/completed/unsubscribe")
{
    auto mock = mock_observer<int>{};
    GIVEN("publish subject")
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        WHEN("emit value")
        {
            subj.get_subscriber().on_next(1);
            AND_WHEN("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                THEN("observer doesn't obtain value")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        WHEN("emit error")
        {
            subj.get_subscriber().on_error(std::make_exception_ptr(std::runtime_error{""}));
            AND_WHEN("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                THEN("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 1);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        WHEN("emit on_completed")
        {
            subj.get_subscriber().on_completed();
            AND_WHEN("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                THEN("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
        WHEN("emit unsubscribe")
        {
            subj.get_subscriber().unsubscribe();
            AND_WHEN("subscribe observer after emission")
            {
                auto sub = subj.get_observable().subscribe(mock);
                THEN("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(sub.is_subscribed() == false);
                }
            }
        }
        WHEN("emit error and on_completed")
        {
            subj.get_subscriber().on_error(std::make_exception_ptr(std::runtime_error{ "" }));
            subj.get_subscriber().on_completed();
            AND_WHEN("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                THEN("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 1);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        WHEN("emit on_completed and error")
        {
            subj.get_subscriber().on_completed();
            subj.get_subscriber().on_error(std::make_exception_ptr(std::runtime_error{ "" }));
            AND_WHEN("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                THEN("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
        WHEN("emit everything after on_completed via get_observer to avoid subscription")
        {
            auto observer = subj.get_subscriber().get_observer();
            observer.on_completed();
            subj.get_observable().subscribe(mock);
            observer.on_next(1);
            observer.on_error(std::make_exception_ptr(std::runtime_error{""}));
            observer.on_completed();
            THEN("no any calls except of cached on_completed")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}