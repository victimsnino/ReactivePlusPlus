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

#include <rpp/observables/connectable_observable.hpp>
#include <rpp/operators/map.hpp>
#include <rpp/operators/multicast.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "mock_observer.hpp"

TEST_CASE("connectable observable")
{
    auto mock = mock_observer_strategy<int>{};
    auto d    = rpp::composite_disposable_wrapper::make();

    SECTION("source and connectable observable from it")
    {
        auto source = rpp::source::just(1);
        auto test   = [&mock, &d](auto&& connectable) {
            SECTION("subscribe on connectable")
            {
                connectable.subscribe(mock.get_observer(d));
                SECTION("nothing happens")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(!d.is_disposed());
                }
                SECTION("call connect")
                {
                    auto sub_connectable = connectable.connect();
                    SECTION("observer obtains values")
                    {
                        CHECK(mock.get_received_values() == std::vector{1});
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                        CHECK(d.is_disposed());
                        CHECK(sub_connectable.is_disposed());
                    }
                }
                SECTION("call connect multiple times")
                {
                    auto sub_connectable = connectable.connect();
                    connectable.connect();
                    SECTION("observer obtains values")
                    {
                        CHECK(mock.get_received_values() == std::vector{1});
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                        CHECK(d.is_disposed());
                        CHECK(sub_connectable.is_disposed());
                    }
                }
            }
            SECTION("subscribe on connecatble via map and connect")
            {
                connectable | rpp::ops::map([](int val) { return val * 10; }) | rpp::ops::subscribe(mock.get_observer(d));
                auto sub_connectable = connectable.connect();
                SECTION("observer obtains modified values")
                {
                    CHECK(mock.get_received_values() == std::vector{10});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                    CHECK(d.is_disposed());
                    CHECK(sub_connectable.is_disposed());
                }
            }
        };
        SECTION("connectable created manually")
        test(rpp::connectable_observable{source, rpp::subjects::publish_subject<int>{}});
        SECTION("connectable created via multicast")
        test(source | rpp::ops::multicast(rpp::subjects::publish_subject<int>{}));
        SECTION("connectable created via templated multicast")
        test(source | rpp::ops::multicast<rpp::subjects::publish_subject>());
    }
    SECTION("subject as source and connectable observable from it")
    {
        auto source = rpp::subjects::publish_subject<int>();
        auto test   = [&mock, &source, &d](auto&& connectable) {
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
                        CHECK(mock.get_received_values() == std::vector{1});
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                        CHECK(!d.is_disposed());
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
                        CHECK(!d.is_disposed());
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
                            CHECK(!d.is_disposed());
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
                        CHECK(d.is_disposed());
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
                            CHECK(d.is_disposed());
                            CHECK(sub_connectable.is_disposed());
                            CHECK(new_sub_connectable.is_disposed());
                        }
                    }
                }
            }
        };
        SECTION("connectable created manually")
        test(rpp::connectable_observable{source.get_observable(), rpp::subjects::publish_subject<int>{}});
        SECTION("connectable created via multicast")
        test(source.get_observable() | rpp::ops::multicast(rpp::subjects::publish_subject<int>{}));
        SECTION("connectable created via templated multicast")
        test(source.get_observable() | rpp::ops::multicast<rpp::subjects::publish_subject>());
    }
    SECTION("observable")
    {
        auto source = rpp::source::just(1);
        // SECTION("call publish on it")
        // {
        //     auto published = source.publish();
        //     SECTION("published observable is same as Connectable with publish_subject")
        //     {
        //         static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<int, rpp::subjects::publish_subject<int>, decltype(source)>>);
        //     }
        // }
        SECTION("call multicast on it with publish_subject")
        {
            auto published = source | rpp::ops::multicast(rpp::subjects::publish_subject<int>{});
            SECTION("published observable is same as Connectable with publish_subject")
            {
                static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<decltype(source), rpp::subjects::publish_subject<int>>>);
            }
        }
        SECTION("call template multicast on it with publish_subject")
        {
            auto published = source | rpp::ops::multicast();
            SECTION("published observable is same as Connectable with publish_subject")
            {
                static_assert(rpp::constraint::decayed_same_as<decltype(published), rpp::connectable_observable<decltype(source), rpp::subjects::publish_subject<int>>>);
            }
        }
    }
}

TEST_CASE("ref_count")
{
    auto observer_1 = mock_observer_strategy<int>{};
    auto observer_2 = mock_observer_strategy<int>{};
    SECTION("connectable observable from just")
    {
        auto observable = rpp::source::just(1) | rpp::ops::multicast();

        SECTION("subscribe on it without ref_count")
        {
            observable.subscribe(observer_1);
            SECTION("nothing happens")
            {
                CHECK(observer_1.get_total_on_next_count() == 0);
                CHECK(observer_1.get_on_error_count() == 0);
                CHECK(observer_1.get_on_completed_count() == 0);
            }
            SECTION("subscribe on it another observer with ref_count")
            {
                observable.ref_count().subscribe(observer_2);
                SECTION("both observers obtain values")
                {
                    auto validate = [](auto observer) {
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
    SECTION("connectable observable from subject")
    {
        auto subj       = rpp::subjects::publish_subject<int>{};
        auto observable = subj.get_observable() | rpp::ops::multicast();

        SECTION("subscribe on it without ref_count and with ref_count")
        {
            observable.subscribe(observer_1);
            auto sub = rpp::composite_disposable_wrapper::make();
            observable.ref_count().subscribe(rpp::composite_disposable_wrapper{sub}, observer_2);
            SECTION("send value")
            {
                subj.get_observer().on_next(1);
                SECTION("both observers obtain values")
                {
                    auto validate = [](auto observer) {
                        CHECK(observer.get_received_values() == std::vector{1});
                        CHECK(observer.get_total_on_next_count() == 1);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 0);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
            }
            SECTION("unsubscribe observer with ref_count and send value")
            {
                sub.dispose();
                subj.get_observer().on_next(1);
                SECTION("no observers obtain values")
                {
                    auto validate = [](auto observer) {
                        CHECK(observer.get_total_on_next_count() == 0);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 0);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
                SECTION("subscribe via ref_count again and send value")
                {
                    sub = rpp::composite_disposable_wrapper::make();
                    observable.ref_count().subscribe(rpp::composite_disposable_wrapper{sub}, observer_2);
                    subj.get_observer().on_next(1);
                    SECTION("both observers obtain values")
                    {
                        auto validate = [](auto observer) {
                            CHECK(observer.get_received_values() == std::vector{1});
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
        SECTION("subscribe both with ref_count")
        {
            observable.ref_count().subscribe(observer_1);
            auto sub = rpp::composite_disposable_wrapper::make();
            observable.ref_count().subscribe(rpp::composite_disposable_wrapper{sub}, observer_2);
            SECTION("send value")
            {
                subj.get_observer().on_next(1);
                SECTION("both observers obtain values")
                {
                    auto validate = [](auto observer) {
                        CHECK(observer.get_received_values() == std::vector{1});
                        CHECK(observer.get_total_on_next_count() == 1);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 0);
                    };
                    validate(observer_1);
                    validate(observer_2);
                }
            }
            SECTION("unsubscribe 1 observer with ref_count and send value")
            {
                sub.dispose();
                subj.get_observer().on_next(1);
                SECTION("first observer obtains values")
                {
                    CHECK(observer_1.get_received_values() == std::vector{1});
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