//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus


#include <rpp/sources/just.hpp>
#include <rpp/operators/ref_count.hpp>
#include <rpp/operators/publish.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <catch2/catch_test_macros.hpp>

#include "mock_observer.hpp"

SCENARIO("ref_count")
{
    auto observer_1 = mock_observer<int>{};
    auto observer_2 = mock_observer<int>{};
    GIVEN("connectable observable from just")
    {
        auto observable = rpp::source::just(1).publish();

        WHEN("subscribe on it without ref_count")
        {
            observable.subscribe(observer_1);
            THEN("nothing happens")
            {
                CHECK(observer_1.get_total_on_next_count() == 0);
                CHECK(observer_1.get_on_error_count() == 0);
                CHECK(observer_1.get_on_completed_count() == 0);
            }
            AND_WHEN("subscribe on it another observer with ref_count")
            {
                observable.ref_count().subscribe(observer_2);
                THEN("both observers obtain values")
                {
                    auto validate = [](auto observer)
                    {
                        CHECK(observer.get_received_values() == std::vector{1});
                        CHECK(observer.get_total_on_next_count() == 1);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 1);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
            }
        }
        
    }
    GIVEN("connectable observable from subject")
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        auto observable = subj.get_observable().publish();

        WHEN("subscribe on it without ref_count and with ref_count")
        {
            observable.subscribe(observer_1);
            auto sub = observable.ref_count().subscribe(observer_2);
            AND_WHEN("send value")
            {
                subj.get_subscriber().on_next(1);
                THEN("both observers obtain values")
                {
                    auto validate = [](auto observer)
                    {
                        CHECK(observer.get_received_values() == std::vector{ 1 });
                        CHECK(observer.get_total_on_next_count() == 1);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 0);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
            }
            AND_WHEN("unsubscribe observr with ref_count and send value")
            {
                sub.unsubscribe();
                subj.get_subscriber().on_next(1);
                THEN("no observers obtain values")
                {
                    auto validate = [](auto observer)
                    {
                        CHECK(observer.get_total_on_next_count() == 0);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 0);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
                AND_WHEN("subscribe via ref_count again and send value")
                {
                    sub = observable.ref_count().subscribe(observer_2);
                    subj.get_subscriber().on_next(1);
                    THEN("both observers obtain values")
                    {
                        auto validate = [](auto observer)
                        {
                            CHECK(observer.get_received_values() == std::vector{ 1 });
                            CHECK(observer.get_total_on_next_count() == 1);
                            CHECK(observer.get_on_error_count() == 0);
                            CHECK(observer.get_on_completed_count() == 0);
                        };
                        validate(observer_1);
                        validate(observer_2);
                    }
                }
            }
        }
        WHEN("subscribe both with ref_count")
        {
            observable.ref_count().subscribe(observer_1);
            auto sub = observable.ref_count().subscribe(observer_2);
            AND_WHEN("send value")
            {
                subj.get_subscriber().on_next(1);
                THEN("both observers obtain values")
                {
                    auto validate = [](auto observer)
                    {
                        CHECK(observer.get_received_values() == std::vector{ 1 });
                        CHECK(observer.get_total_on_next_count() == 1);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 0);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
            }
            AND_WHEN("unsubscribe 1 observr with ref_count and send value")
            {
                sub.unsubscribe();
                subj.get_subscriber().on_next(1);
                THEN("first observer obtains values")
                {
                    CHECK(observer_1.get_received_values() == std::vector{ 1 });
                    CHECK(observer_1.get_total_on_next_count() == 1);
                    CHECK(observer_1.get_on_error_count() == 0);
                    CHECK(observer_1.get_on_completed_count() == 0);

                    CHECK(observer_2.get_total_on_next_count() == 0);
                    CHECK(observer_2.get_on_error_count() == 0);
                    CHECK(observer_2.get_on_completed_count() == 0);
                }
            }
        }
    }
}
