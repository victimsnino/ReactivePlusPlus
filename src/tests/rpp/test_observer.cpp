//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include "copy_count_tracker.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/observers.hpp>

template<typename T>
void validate_observer(const rpp::interface_observer<T>& obs,
                       const size_t&                     on_next_count,
                       const size_t&                     on_err_count,
                       const size_t&                     on_completed_count)
{
    WHEN("call on_next")
    {
        obs.on_next(1);
        THEN("on_next callback called")
        {
            CHECK(on_next_count == 1);
            CHECK(on_err_count == 0);
            CHECK(on_completed_count == 0);
        }
        AND_WHEN("call on_next one more")
        {
            obs.on_next(1);
            THEN("on_next callback called twice")
            {
                CHECK(on_next_count == 2);
                CHECK(on_err_count == 0);
                CHECK(on_completed_count == 0);
            }
        }
        AND_WHEN("call on_error")
        {
            obs.on_error(std::exception_ptr{});
            THEN("on_error callback called")
            {
                CHECK(on_next_count == 1);
                CHECK(on_err_count == 1);
                CHECK(on_completed_count == 0);
            }
            AND_WHEN("call on_next")
            {
                obs.on_next(1);
                THEN("on_next callback not called")
                {
                    CHECK(on_next_count == 1);
                    CHECK(on_err_count == 1);
                    CHECK(on_completed_count == 0);
                }
            }
        }
        AND_WHEN("call on_completed")
        {
            obs.on_completed();
            THEN("on_completed callback called")
            {
                CHECK(on_next_count == 1);
                CHECK(on_err_count == 0);
                CHECK(on_completed_count == 1);
            }
             AND_WHEN("call on_next")
            {
                obs.on_next(1);
                THEN("on_next callback not called")
                {
                    CHECK(on_next_count == 1);
                    CHECK(on_err_count == 0);
                    CHECK(on_completed_count == 1);
                }
            }
        }
    }
}

SCENARIO("anonymous observer calls provided actions but keeps termination", "[observer]")
{
    GIVEN("anonymous_observer")
    {
        size_t on_next_count{};
        size_t on_err_count{};
        size_t on_completed_count{};

        auto obs = rpp::anonymous_observer{[&](int) { ++on_next_count; },
                                           [&](const std::exception_ptr&) { ++on_err_count; },
                                           [&]() { ++on_completed_count; }};

        static_assert(!std::is_copy_constructible_v<decltype(obs)>, "anonymous observer should be not copy constructible");
        static_assert(std::move_constructible<decltype(obs)>, "anonymous observer should be move constructible");

        validate_observer(obs, on_next_count, on_err_count, on_completed_count);
    }
}

SCENARIO("anonymous observer can be casted to dynamic_observer and still obtain callbacks", "[observer]")
{
    GIVEN("anonymous_observer")
    {
        size_t on_next_count{};
        size_t on_err_count{};
        size_t on_completed_count{};

        auto obs = rpp::anonymous_observer{[&](int) { ++on_next_count; },
                                           [&](const std::exception_ptr&) { ++on_err_count; },
                                           [&]() { ++on_completed_count; }};

        WHEN("call as_dynamic")
        {
            auto dynamic = obs.as_dynamic();
            static_assert(std::is_copy_constructible_v<decltype(dynamic)>, "dynamic observer should be copy constructible");
            static_assert(std::move_constructible<decltype(dynamic)>, "anonymous observer should be move constructible");

            THEN("dynamic observer obtained")
            {
                static_assert(std::is_same_v<decltype(dynamic), rpp::dynamic_observer<int>>);
            }
            THEN("dynamic observer forwards calls to original observer")
            {
                validate_observer(dynamic, on_next_count, on_err_count, on_completed_count);
            }
            AND_WHEN("copy dynamic observer")
            {
                THEN("copy of dynamic observer still forwards calls to original observer")
                {
                    const auto copy_of_dynamic = dynamic;
                    validate_observer(copy_of_dynamic, on_next_count, on_err_count, on_completed_count);
                }
            }
        }
    }
}

SCENARIO("as_dynamic call for anonymous observer doesn't provide a lot of overhead", "[observer]")
{
    GIVEN("anonymous_observer")
    {
        copy_count_tracker tracker{};

        auto obs = rpp::anonymous_observer{[tracker](int) { }};

        const auto initial_copy = tracker.get_copy_count();
        const auto initial_move = tracker.get_move_count();

        WHEN("call as_dynamic as lvalue")
        {
            auto dynamic = obs.as_dynamic();
            THEN("only one copy happens")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 1);
                CHECK(tracker.get_move_count() - initial_move == 0);
            }
        }
        WHEN("call as_dynamic as rvalue")
        {
            auto dynamic = std::move(obs).as_dynamic();
            THEN("only one move happens")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 0);
                CHECK(tracker.get_move_count() - initial_move == 1);
            }
        }
    }
}
