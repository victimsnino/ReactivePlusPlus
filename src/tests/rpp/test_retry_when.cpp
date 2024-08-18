//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/retry_when.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>

#include "copy_count_tracker.hpp"
#include "disposable_observable.hpp"


TEST_CASE("retry_when resubscribes on notifier emission")
{
    auto observer = mock_observer_strategy<std::string>();
    SECTION("observable without error emission")
    {
        size_t subscribe_count = 0;
        auto   observable      = rpp::source::create<std::string>([&subscribe_count](const auto& sub) {
            ++subscribe_count;
            sub.on_next(std::string{"1"});
            sub.on_completed();
        });
        SECTION("subscribe")
        {
            observable
                | rpp::operators::retry_when([](const std::exception_ptr&) { return rpp::source::just(1); })
                | rpp::operators::subscribe(observer);
            SECTION("observer obtains values from observable")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 1);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
    }

    SECTION("observable with 1 error")
    {
        size_t subscribe_count = 0;
        auto   observable      = rpp::source::create<std::string>([&subscribe_count](const auto& sub) {
            if (subscribe_count++ == 0)
            {
                sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
            }
            else
            {
                sub.on_next(std::string{"1"});
                sub.on_completed();
            }
        });

        SECTION("subscribe on it with single emission notifier")
        {
            observable
                | rpp::operators::retry_when([](const std::exception_ptr&) { return rpp::source::just(1); })
                | rpp::operators::subscribe(observer);
            SECTION("original observable is subscribed twice and observer receives one emission")
            {
                CHECK(subscribe_count == 2);
                CHECK(observer.get_total_on_next_count() == 1);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }

        SECTION("subscribe on it with multiple emissions notifier")
        {
            observable
                | rpp::operators::retry_when([](const std::exception_ptr&) { return rpp::source::just(1, 2, 3); })
                | rpp::operators::subscribe(observer);
            SECTION("original observable is subscribed twice and observer receives only one emission")
            {
                CHECK(subscribe_count == 2);
                CHECK(observer.get_total_on_next_count() == 1);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }

        SECTION("subscribe on it with throwing notifier")
        {
            observable
                | rpp::operators::retry_when([](const std::exception_ptr&) {
                      throw std::runtime_error{"error"};
                      return rpp::source::just(1);
                  })
                | rpp::operators::subscribe(observer);
            SECTION("original observable is subscribed only once and observer receives error emission")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }

        SECTION("subscribe on it with error notifier")
        {
            observable
                | rpp::operators::retry_when([](const std::exception_ptr&) {
                      return rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{"error"}));
                  })
                | rpp::operators::subscribe(observer);
            SECTION("original observable is subscribed only once and observer receives error emission")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 1);
                CHECK(observer.get_on_completed_count() == 0);
            }
        }

        SECTION("subscribe on it with empty notifier")
        {
            observable
                | rpp::operators::retry_when([](const std::exception_ptr&) {
                      return rpp::source::empty<int>();
                  })
                | rpp::operators::subscribe(observer);
            SECTION("original observable is subscribed only once and observer receives completed emission")
            {
                CHECK(subscribe_count == 1);
                CHECK(observer.get_total_on_next_count() == 0);
                CHECK(observer.get_on_error_count() == 0);
                CHECK(observer.get_on_completed_count() == 1);
            }
        }
    }
}

TEST_CASE("retry_when doesn't produce extra copies")
{
    SECTION("retry_when(empty_notifier)")
    {
        copy_count_tracker::test_operator(rpp::ops::retry_when([](const std::exception_ptr&) { return rpp::source::empty<int>(); }),
                                          {
                                              .send_by_copy = {.copy_count = 1, // 1 copy to final subscriber
                                                               .move_count = 0},
                                              .send_by_move = {.copy_count = 0,
                                                               .move_count = 1} // 1 move to final subscriber
                                          });
    }
}

TEST_CASE("retry_when satisfies disposable contracts")
{
    auto observable_disposable = rpp::composite_disposable_wrapper::make();
    {
        auto observable = observable_with_disposable<int>(observable_disposable);
        auto op         = rpp::ops::retry_when([](const std::exception_ptr&) { return rpp::source::empty<int>(); });

        test_operator_with_disposable<int>(op);
        test_operator_finish_before_dispose<int>(op);
    }

    CHECK((observable_disposable.is_disposed() || observable_disposable.lock().use_count() == 2));
}
