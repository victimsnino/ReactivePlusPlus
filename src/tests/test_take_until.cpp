//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//                     TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/take_until.hpp>
#include <rpp/schedulers/new_thread_scheduler.hpp>
#include <rpp/schedulers/trampoline_scheduler.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/interval.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "mock_observer.hpp"

SCENARIO("take_until mirrors both source observable and trigger observable", "[take_until]")
{
    GIVEN("observable of -1-1... pairs with trigger observable from a publish subject")
    {
        auto subject = rpp::subjects::publish_subject<bool>{};
        auto mock = mock_observer<int>{};

        rpp::source::create<int>([other_subscriber = subject.get_subscriber()](const auto& subscriber)
                                 {
                                    subscriber.on_next(1);
                                    subscriber.on_next(2);

                                    // Should see a terminate event after this
                                    other_subscriber.on_next(true);

                                    subscriber.on_next(3);
                                 })
            .take_until(subject.get_observable())
            .subscribe(mock);

        THEN("should see -1-2-|")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1, 2});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -1-2-3-| pairs with trigger observable of never")
    {
        auto mock = mock_observer<int>{};
        rpp::source::just(1, 2, 3)
            .take_until(rpp::source::never<bool>())
            .subscribe(mock);

        THEN("should see -1-2-3-|")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1, 2, 3});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -1-| pairs with trigger observable of -1-|")
    {
        auto mock = mock_observer<int>{};
        rpp::source::just(1)
            .take_until(rpp::source::just(1))
            .subscribe(mock);

        THEN("should see -| because take_until is subscribed first and it also mirrors the trigger observable 's completed event")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -1-| pairs with trigger observable of -x")
    {
        auto mock = mock_observer<int>{};

        rpp::source::just(1)
            .take_until(rpp::source::error<bool>(std::make_exception_ptr(std::runtime_error{""})))
            .subscribe(mock);

        THEN("should see -x because take_until also mirrors the trigger observable 's error event")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }
}

SCENARIO("take_until can handle race condition")
{
    GIVEN("source observable and trigger observable emits in different thread")
    {
        std::atomic_bool on_completed_called{false};

        rpp::source::interval(std::chrono::seconds{1}, rpp::schedulers::trampoline{})
            .take_until(rpp::source::interval(std::chrono::seconds{2}, rpp::schedulers::new_thread{}))
            .subscribe([&](const auto&)
                       {
                           CHECK(!on_completed_called);
                           std::this_thread::sleep_for(std::chrono::seconds(3));
                           CHECK(!on_completed_called);
                       } /* on_next */,
                       {} /* on_error */,
                       [&]()
                       {
                           on_completed_called = true;
                       } /* on_completed */);

        CHECK(on_completed_called);
    }
}
