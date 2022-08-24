//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include <catch2/catch_test_macros.hpp>

#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/operators/do.hpp>

#include "mock_observer.hpp"

static auto get_on_next(std::stringstream& callstack, std::string prefix)
{
    return [&, prefix](int v) { callstack << prefix << "_on_next " << v << "\n"; };
}

static auto get_on_error(std::stringstream& callstack, std::string prefix)
{
    return [&, prefix](std::exception_ptr) { callstack << prefix << "_on_error\n"; };
}

static auto get_on_completed(std::stringstream& callstack, std::string prefix)
{
    return [&, prefix]() { callstack << prefix << "_on_completed\n"; };
}

SCENARIO("tap invokes callbacks for tapped observer", "[operators][do]")
{
    std::string tapped_prefix = "tapped";
    std::string external_prefix = "external";
    std::stringstream callstack{};
    auto              external_observer = rpp::make_specific_observer<int>(get_on_next(callstack, external_prefix), get_on_error(callstack, external_prefix), get_on_completed(callstack, external_prefix));

    auto validate = [&](const auto& ...tap_args)
    {
        AND_GIVEN("observable of values")
        {
            auto obs = rpp::source::just(1, 2);
            WHEN("subscribe on it via tap")
            {
                obs.tap(tap_args...).subscribe(external_observer);
                THEN("both (tapped and original) observers see values while tapped see earlier")
                {
                    std::stringstream expected{};
                    get_on_next(expected, tapped_prefix)(1);
                    get_on_next(expected, external_prefix)(1);
                    get_on_next(expected, tapped_prefix)(2);
                    get_on_next(expected, external_prefix)(2);
                    get_on_completed(expected, tapped_prefix)();
                    get_on_completed(expected, external_prefix)();
                    CHECK(callstack.str() == expected.str());
                }
            }
        }

        AND_GIVEN("observable with error")
        {
            auto obs = rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{ "" }));
            WHEN("subscribe on it via tap")
            {
                obs.tap(tap_args...).subscribe(external_observer);
                THEN("both (tapped and original) observers see errors while tapped see earlier")
                {
                    std::stringstream expected{};
                    get_on_error(expected, tapped_prefix)(std::exception_ptr{});
                    get_on_error(expected, external_prefix)(std::exception_ptr{});
                    CHECK(callstack.str() == expected.str());
                }
            }
        }

        AND_GIVEN("observable with never")
        {
            auto obs = rpp::source::never<int>();
            WHEN("subscribe on it via tap")
            {
                obs.tap(tap_args...).subscribe(external_observer);
                THEN("both (tapped and original) observers see nothing")
                {
                    std::stringstream expected{};
                    CHECK(callstack.str() == expected.str());
                }
            }
        }

        AND_GIVEN("observable with empty")
        {
            auto obs = rpp::source::empty<int>();
            WHEN("subscribe on it via tap")
            {
                obs.tap(tap_args...).subscribe(external_observer);
                THEN("both (tapped and original) observers see completed while tapped see earlier")
                {
                    std::stringstream expected{};
                    get_on_completed(expected, tapped_prefix)();
                    get_on_completed(expected, external_prefix)();
                    CHECK(callstack.str() == expected.str());
                }
            }
        }
    };

    GIVEN("observer as argument for tap")
        validate(rpp::make_specific_observer<int>(get_on_next(callstack, tapped_prefix), get_on_error(callstack, tapped_prefix), get_on_completed(callstack, tapped_prefix)));

    GIVEN("callbacks list as arguments for tap")
        validate(get_on_next(callstack, tapped_prefix), get_on_error(callstack, tapped_prefix), get_on_completed(callstack, tapped_prefix));
}

SCENARIO("do_on_next invokes on_next callback", "[operators][do]")
{
    std::vector<int> items{};
    auto on_next = [&](int v) {items.push_back(v); };

    GIVEN("observable of items")
    {
        const auto obs = rpp::source::just(1, 2);
        WHEN("subscribe on it via do_on_next")
        {
            obs.do_on_next(on_next).subscribe();
            THEN("callback sees new items")
            {
                CHECK(items == std::vector{ 1,2 });
            }
        }
    }
}

SCENARIO("do_on_error invokes on_error callback", "[operators][do]")
{
    size_t on_error_count{};
    auto on_error = [&](const auto&) {++on_error_count; };

    GIVEN("observable with error")
    {
        const auto obs = rpp::source::error<int>(std::exception_ptr{});
        WHEN("subscribe on it via do_on_next")
        {
            obs.do_on_error(on_error).subscribe(rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{});
            THEN("callback sees error")
            {
                CHECK(on_error_count == 1);
            }
        }
    }
}

SCENARIO("do_on_completed invokes on_completed callback", "[operators][do]")
{
    size_t on_completed_count{};
    auto on_completed = [&]() {++on_completed_count; };

    GIVEN("observable with completed")
    {
        const auto obs = rpp::source::empty<int>();
        WHEN("subscribe on it via do_on_completed")
        {
            obs.do_on_completed(on_completed).subscribe();
            THEN("callback sees completed")
            {
                CHECK(on_completed_count == 1);
            }
        }
    }
}