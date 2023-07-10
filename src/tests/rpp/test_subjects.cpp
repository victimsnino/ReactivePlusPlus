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

#include <rpp/subjects/publish_subject.hpp>

#include "mock_observer.hpp"

TEST_CASE("publish subject multicasts values")
{
    auto mock_1 = mock_observer_strategy<int>{};
    auto mock_2 = mock_observer_strategy<int>{};
    SECTION("publish subject")
    {
        auto sub = rpp::subjects::publish_subject<int>{};
        SECTION("subscribe multiple observers")
        {
            sub.get_observable().subscribe(mock_1.get_observer());
            sub.get_observable().subscribe(mock_2.get_observer());

            SECTION("emit value")
            {
                sub.get_observer().on_next(1);
                SECTION("observers obtain value")
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
            SECTION("emit error")
            {
                sub.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
                SECTION("observers obtain error")
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
            SECTION("emit on_completed")
            {
                sub.get_observer().on_completed();
                SECTION("observers obtain on_completed")
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
            SECTION("emit unsubscribe")
            {
                sub.get_observer().unsubscribe();
                SECTION("observers obtain on_completed")
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
            SECTION("emit multiple values")
            {
                SECTION("each sbuscriber obtain first value, SECTION seconds and etc")
                {
                    sub.get_observer().on_next(1);
                    auto check_1 = [](auto mock) { CHECK(mock.get_received_values() == std::vector{ 1 }); };
                    check_1(mock_1);
                    check_1(mock_2);

                    sub.get_observer().on_next(2);
                    auto check_2 = [](auto mock) { CHECK(mock.get_received_values() == std::vector{ 1,2 }); };
                    check_2(mock_1);
                    check_2(mock_2);
                }
            }
            SECTION("first subscriber unsubscribes and SECTION emit value")
            {
                sub_1.unsubscribe();
                sub.get_observer().on_next(1);
                SECTION("observers obtain value")
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

TEST_CASE("publish subject caches error/completed/unsubscribe")
{
    auto mock = mock_observer_strategy<int>{};
    SECTION("publish subject")
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        SECTION("emit value")
        {
            subj.get_observer().on_next(1);
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock.get_observer());
                SECTION("observer doesn't obtain value")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("emit error")
        {
            subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock.get_observer());
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 1);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("emit on_completed")
        {
            subj.get_observer().on_completed();
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock.get_observer());
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
        SECTION("emit unsubscribe")
        {
            subj.get_observer().unsubscribe();
            SECTION("subscribe observer after emission")
            {
                auto sub = subj.get_observable().subscribe(mock.get_observer());
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                    CHECK(sub.is_subscribed() == false);
                }
            }
        }
        SECTION("emit error and on_completed")
        {
            subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{ "" }));
            subj.get_observer().on_completed();
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock.get_observer());
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 1);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("emit on_completed and error")
        {
            subj.get_observer().on_completed();
            subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{ "" }));
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock.get_observer());
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
        SECTION("emit everything after on_completed via get_observer to avoid subscription")
        {
            auto observer = subj.get_observer();
            observer.on_completed();
            subj.get_observable().subscribe(mock.get_observer());
            observer.on_next(1);
            observer.on_error(std::make_exception_ptr(std::runtime_error{""}));
            observer.on_completed();
            SECTION("no any calls except of cached on_completed")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}