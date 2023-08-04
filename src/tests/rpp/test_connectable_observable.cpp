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

#include "mock_observer.hpp"

#include <rpp/observables/connectable_observable.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/subjects/publish_subject.hpp>
#include <rpp/operators/map.hpp>

TEST_CASE("connectable observable")
{
    auto mock = mock_observer_strategy<int>{};
    auto d = std::make_shared<rpp::composite_disposable>();

    SECTION("source and connectable observable from it")
    {
        auto source = rpp::source::just(1);
        auto connectable = rpp::connectable_observable{ source, rpp::subjects::publish_subject<int>{} };
        SECTION("subscribe on connectable")
        {
            connectable.subscribe(mock.get_observer(d));
            SECTION("nothing happens")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
                CHECK(!d->is_disposed());
            }
            SECTION("call connect")
            {
                auto sub_connectable = connectable.connect();
                SECTION("observer obtains values")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(d->is_disposed());
                    CHECK(sub_connectable.is_disposed());
                }
            }
            SECTION("call connect multiple times")
            {
                auto sub_connectable = connectable.connect();
                connectable.connect();
                SECTION("observer obtains values")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(d->is_disposed());
                    CHECK(sub_connectable.is_disposed());
                }
            }
        }
        SECTION("subscribe on connecatble via map and connect")
        {
            connectable | rpp::ops::map([](int val) {return val * 10; }) | rpp::ops::subscribe(mock.get_observer(d));
            auto sub_connectable = connectable.connect();
            SECTION("observer obtains modified values")
            {
                CHECK(mock.get_received_values() == std::vector{ 10 });
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(d->is_disposed());
                CHECK(sub_connectable.is_disposed());
            }
        }
    }
    SECTION("subject as source and connectable observable from it")
    {
        auto source = rpp::subjects::publish_subject<int>();
        auto connectable = rpp::connectable_observable{ source.get_observable(), rpp::subjects::publish_subject<int>{} };
        SECTION("subscribe on connectable and connect")
        {
            connectable.subscribe(mock.get_observer(d));
            auto sub_connectable = connectable.connect();
            SECTION("call connect again and send value")
            {
                auto new_sub_connectable = connectable.connect();
                source.get_observer().on_next(1);

                SECTION("observer obtains values only once")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1 });
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(!d->is_disposed());
                    CHECK(!sub_connectable.is_disposed());
                }
            }
            SECTION("unsubscribe connected subscription before any values from source")
            {
                sub_connectable.dispose();
                source.get_observer().on_next(1);
                SECTION("subscriber obtains nothing")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(!d->is_disposed());
                    CHECK(sub_connectable.is_disposed());
                    CHECK(!source.get_disposable().is_disposed());
                }
                SECTION("connect again and send values")
                {
                    auto new_sub_connectable = connectable.connect();
                    source.get_observer().on_next(1);

                    SECTION("subscriber obtains values")
                    {
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                        CHECK(!d->is_disposed());
                        CHECK(sub_connectable.is_disposed());
                        CHECK(!source.get_disposable().is_disposed());
                    }
                }
            }
            SECTION("obtain on_completed")
            {
                source.get_observer().on_completed();
                SECTION("subscribe obtains on_completed and unsubscribe initiated")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(d->is_disposed());
                    CHECK(sub_connectable.is_disposed());
                }
                SECTION("connect again and send values")
                {
                    auto new_sub_connectable = connectable.connect();
                    source.get_observer().on_next(1);
                    SECTION("subscriber obtains nothing")
                    {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                        CHECK(d->is_disposed());
                        CHECK(sub_connectable.is_disposed());
                        CHECK(new_sub_connectable.is_disposed());
                    }
                }
            }
        }
    }
    // SECTION("observable")
    // {
    //     auto source = rpp::source::just(1);
    //     SECTION("call publish on it")
    //     {
    //         auto published = source.publish();
    //         SECTION("published observable is same as Connectable with publish_subject")
    //         {
    //             static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<int, rpp::subjects::publish_subject<int>, decltype(source)>>);
    //         }
    //     }
    //     SECTION("call multicast on it with publish_subject")
    //     {
    //         auto published = source.multicast(rpp::subjects::publish_subject <int>{});
    //         SECTION("published observable is same as Connectable with publish_subject")
    //         {
    //             static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<int, rpp::subjects::publish_subject<int>, decltype(source)>>);
    //         }
    //     }
    // }
}