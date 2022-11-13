//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//                     TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/concat.hpp>
#include <rpp/operators/on_error_resume_next.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>

SCENARIO("on_error_resume_next captures error by subscribing to new observable", "[on_error_resume_next]")
{
    GIVEN("observable of -x")
    {
        auto mock = mock_observer<int>{};

        rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
                .on_error_resume_next([](auto&&)
                {
                    return rpp::observable::just(1, 2, 3);
                })
                .subscribe(mock);

        THEN("should see -1-2-3-|")
        {
            CHECK(mock.get_received_values() == std::vector{1, 2, 3});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -x and new observable emits error")
    {
        auto mock = mock_observer<int>{};

        rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
                .on_error_resume_next([](auto&&)
                {
                    return rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}));
                })
                .subscribe(mock);

        THEN("should see -x")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }

    GIVEN("observable of -x and new observable completes immediately")
    {
        auto mock = mock_observer<int>{};

        rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
                .on_error_resume_next([](auto&&)
                {
                    return rpp::source::empty<int>();
                })
                .subscribe(mock);

        THEN("should see -|")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -1-2-x and new observable completes immediately")
    {
        auto mock = mock_observer<int>{};

        rpp::source::just(rpp::source::just(1).as_dynamic(),
                          rpp::source::just(2).as_dynamic(),
                          rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic())
                .concat()
                .on_error_resume_next([](auto&&)
                {
                    return rpp::source::just(3);
                })
                .subscribe(mock);

        THEN("should see -1-2-3-|")
        {
            CHECK(mock.get_received_values() == std::vector{1, 2, 3});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }
}

SCENARIO("on_error_resume_next forwards on_next and on_completed", "[on_error_resume_next]")
{
    GIVEN("observable of -1-2-3-|")
    {
        auto mock = mock_observer<int>{};

        rpp::source::just(1, 2, 3)
                .on_error_resume_next([](auto&&)
                {
                    return rpp::observable::just(4);
                })
                .subscribe(mock);

        THEN("should see -1-2-3-|")
        {
            CHECK(mock.get_received_values() == std::vector{1, 2, 3});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    GIVEN("observable of -|")
    {
        auto mock = mock_observer<int>{};

        rpp::source::empty<int>()
                .on_error_resume_next([](auto&&)
                {
                    return rpp::observable::just(1);
                })
                .subscribe(mock);

        THEN("should see -|")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }
}
