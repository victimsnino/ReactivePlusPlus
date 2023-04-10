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

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"

#include <rpp/operators/details/strategy.hpp>
#include <rpp/sources/create.hpp>

struct test_strategy
{
    constexpr static rpp::operators::details::forwarding_on_next_strategy on_next{};
    constexpr static rpp::operators::details::forwarding_on_error_strategy on_error{};
    constexpr static rpp::operators::details::forwarding_on_completed_strategy on_completed{};
    constexpr static rpp::operators::details::forwarding_set_upstream_strategy set_upstream{};
    constexpr static rpp::operators::details::forwarding_is_disposed_strategy is_disposed{};
};

TEST_CASE("operator_base_strategy works as expected")
{
    copy_count_tracker tracker{};
    mock_observer_strategy<int> mock{};

    auto observer = rpp::make_lambda_observer([tracker, &mock](int v){ mock.on_next(v); }, [](const std::exception_ptr&){}, [](){});
    auto initial_copies = tracker.get_copy_count();
    auto initial_moves = tracker.get_move_count();
    rpp::operators::details::operator_strategy_base<decltype(observer), test_strategy> op_strategy{std::move(observer)};

    SECTION("operator_strategy delays actual move of observer till move")
    {
        CHECK(tracker.get_copy_count() == initial_copies);
        CHECK(tracker.get_move_count() == initial_moves);

        op_strategy.on_next(1);
        CHECK(mock.get_received_values() == std::vector{1});

        auto moved = std::move(op_strategy);

        int val{2};
        moved.on_next(val);
        CHECK(mock.get_received_values() == std::vector{1, val});

        CHECK(tracker.get_copy_count() == initial_copies);
        CHECK(tracker.get_move_count() == initial_moves + 1);
    }
}