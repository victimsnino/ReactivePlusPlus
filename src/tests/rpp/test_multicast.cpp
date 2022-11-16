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
#include <rpp/rpp.hpp>

SCENARIO("connectable observable")
{
    auto mock = mock_observer<int>{};
    GIVEN("source and connectable observable from it")
    {
        auto source = rpp::source::just(1);
        auto connectable = rpp::connectable_observable{ source, rpp::subjects::publish_subject<int>{} };
        WHEN("subscribe on connectable")
        {
            auto sub = connectable.subscribe(mock);
            THEN("nothing happens")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(sub.is_subscribed());
            }
            AND_WHEN("call connect")
            {
                auto sub_connectable = connectable.connect();
                THEN("observer obtains values")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(sub.is_subscribed() == false);
                    CHECK(sub_connectable.is_subscribed() == false);
                }
            }
            AND_WHEN("call connect multiple times")
            {
                auto sub_connectable = connectable.connect();
                connectable.connect();
                THEN("observer obtains values")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(sub.is_subscribed() == false);
                    CHECK(sub_connectable.is_subscribed() == false);
                }
            }
        }
        WHEN("subscribe on connecatble via map and connect")
        {
            auto sub = connectable.map([](int val) {return val * 10; }).subscribe(mock);
            auto sub_connectable = connectable.connect();
            THEN("observer obtains modified values")
            {
                CHECK(mock.get_received_values() == std::vector{ 10 });
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(sub.is_subscribed() == false);
                CHECK(sub_connectable.is_subscribed() == false);
            }
        }
    }
    GIVEN("subject as source and connectable observable from it")
    {
        auto source = rpp::subjects::publish_subject<int>();
        auto connectable = rpp::connectable_observable{ source.get_observable(), rpp::subjects::publish_subject<int>{} };
        WHEN("subscribe on connectable and connect")
        {
            auto sub = connectable.subscribe(mock);
            auto sub_connectable = connectable.connect();
            AND_WHEN("call connect again and send value")
            {
                auto new_sub_connectable = connectable.connect();
                source.get_subscriber().on_next(1);

                THEN("observer obtains values only once")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(sub.is_subscribed() == true);
                    CHECK(sub_connectable.is_subscribed() == true);
                }
            }
            AND_WHEN("unsubscribe connected subscription before any values from source")
            {
                sub_connectable.unsubscribe();
                source.get_subscriber().on_next(1);
                THEN("subscriber obtains nothing")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(sub.is_subscribed());
                    CHECK(!sub_connectable.is_subscribed());
                    CHECK(source.get_subscriber().is_subscribed());
                }
                AND_WHEN("connect again and send values")
                {
                    auto new_sub_connectable = connectable.connect();
                    source.get_subscriber().on_next(1);

                    THEN("subscriber obtains values")
                    {
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                        CHECK(sub.is_subscribed());
                        CHECK(new_sub_connectable.is_subscribed());
                        CHECK(source.get_subscriber().is_subscribed());
                    }
                }
            }
            AND_WHEN("obtain on_completed")
            {
                // hack but dont break subscription of this subject
                source.get_subscriber().get_observer().on_completed();
                THEN("subscribe obtains on_completed and unsubscribe initiated")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(!sub.is_subscribed());
                    CHECK(!sub_connectable.is_subscribed());
                }
                AND_WHEN("connect again and send values")
                {
                    auto new_sub_connectable = connectable.connect();
                    source.get_subscriber().on_next(1);
                    THEN("subscriber obtains nothing")
                    {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                        CHECK(!sub.is_subscribed());
                        CHECK(!sub_connectable.is_subscribed());
                        CHECK(!new_sub_connectable.is_subscribed());
                    }
                }
            }
        }
    }
    GIVEN("observable")
    {
        auto source = rpp::source::just(1);
        WHEN("call publish on it")
        {
            auto published = source.publish();
            THEN("published observable is same as Connectable with publish_subject")
            {
                static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<int, rpp::subjects::publish_subject<int>, decltype(source)>>);
            }
        }
        WHEN("call multicast on it with publish_subject")
        {
            auto published = source.multicast(rpp::subjects::publish_subject <int>{});
            THEN("published observable is same as Connectable with publish_subject")
            {
                static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<int, rpp::subjects::publish_subject<int>, decltype(source)>>);
            }
        }
    }
}