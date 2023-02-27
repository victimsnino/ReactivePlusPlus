//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_test_macros.hpp>
#include <rpp/observers.hpp>

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
}
