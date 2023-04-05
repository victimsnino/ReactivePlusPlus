//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>

#include "copy_count_tracker.hpp"

#include <rpp/operators/details/strategy.hpp>
#include <rpp/sources/create.hpp>

struct test_strategy : public rpp::operators::details::forwarding_on_next_strategy,
                       public rpp::operators::details::forwarding_on_error_strategy,
                       public rpp::operators::details::forwarding_on_completed_strategy,
                       public rpp::operators::details::forwarding_disposable_strategy
{
};

TEST_CASE("operator_base_strategy works as expected")
{
    copy_count_tracker tracker{};

    auto observer = rpp::make_lambda_observer([tracker](int){}, [](const std::exception_ptr&){}, [](){});
    auto initial_copies = tracker.get_copy_count();
    auto initial_moves = tracker.get_move_count();
    rpp::operators::details::operator_strategy_base<decltype(observer), test_strategy> op_strategy{std::move(observer)};

    SECTION("operator_strategy delays actual move of observer till move") 
    {
        CHECK(tracker.get_copy_count() == initial_copies);
        CHECK(tracker.get_move_count() == initial_moves);

        auto moved = std::move(op_strategy);

        CHECK(tracker.get_copy_count() == initial_copies);
        CHECK(tracker.get_move_count() == initial_moves + 1);
    }
}