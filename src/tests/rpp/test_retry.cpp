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

#include <rpp/operators/retry.hpp>
#include <rpp/sources/concat.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>

#include "copy_count_tracker.hpp"
#include "disposable_observable.hpp"
#include "rpp_trompeloil.hpp"

TEST_CASE("retry handles errors properly")
{
    mock_observer<int>    mock{};
    trompeloeil::sequence seq;

    SECTION("observable 1-x-2")
    {
        const auto observable = rpp::source::concat(rpp::source::just(1), rpp::source::error<int>({}), rpp::source::just(2));

        SECTION("retry(0)")
        {
            REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
            REQUIRE_CALL(*mock, on_error(trompeloeil::_)).IN_SEQUENCE(seq);

            observable | rpp::operators::retry(0) | rpp::operators::subscribe(mock);
        }

        SECTION("retry(1)")
        {
            REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
            REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
            REQUIRE_CALL(*mock, on_error(trompeloeil::_)).IN_SEQUENCE(seq);

            observable | rpp::operators::retry(1) | rpp::operators::subscribe(mock);
        }

        SECTION("retry(2)")
        {
            REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
            REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
            REQUIRE_CALL(*mock, on_next_lvalue(1)).IN_SEQUENCE(seq);
            REQUIRE_CALL(*mock, on_error(trompeloeil::_)).IN_SEQUENCE(seq);

            observable | rpp::operators::retry(2) | rpp::operators::subscribe(mock);
        }
    }
}

TEST_CASE("retry handles stack overflow")
{
    mock_observer<int>    mock{};
    trompeloeil::sequence seq;

    constexpr size_t count = 500000;

    REQUIRE_CALL(*mock, on_next_rvalue(trompeloeil::_)).TIMES(count + 1).IN_SEQUENCE(seq);
    REQUIRE_CALL(*mock, on_error(trompeloeil::_)).IN_SEQUENCE(seq);

    rpp::source::create<int>([](const auto& obs) {
        obs.on_next(1);
        obs.on_error({});
    })
        | rpp::operators::retry(count)
        | rpp::operators::subscribe(mock);
}
