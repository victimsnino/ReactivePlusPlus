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

#include <rpp/operators/distinct_until_changed.hpp>
#include <rpp/sources/just.hpp>

#include "mock_observer.hpp"
#include "copy_count_tracker.hpp"

TEMPLATE_TEST_CASE("distinct_until_changed filters out consecutive duplicates and send first value from duplicates", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    auto mock = mock_observer_strategy<int>{};
    auto obs = rpp::source::just<TestType>(1, 1, 2, 2, 3, 2, 2, 1);
    SECTION("SECTION subscribe on observable with duplicates via distinct_until_changed SECTION subscriber obtains values without consecutive duplicates")
    {
        obs | rpp::ops::distinct_until_changed() | rpp::ops::subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{ 1,2,3,2,1 });
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
    SECTION("SECTION subscribe on observable with duplicates via distinct_until_changed with custom comparator SECTION subscriber obtains values without consecutive duplicates")
    {
        auto op = rpp::ops::distinct_until_changed([](int old_value, int new_value) {return old_value % 2 != new_value % 2; });
        obs | op | rpp::ops::subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{ 1, 1, 3, 1});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
    }
}

TEST_CASE("distinct_until_changed doesn't produce extra copies")
{
    copy_count_tracker::test_operator(rpp::ops::distinct_until_changed(),
                                      {
                                          .send_by_copy = {.copy_count = 2, // 1 copy to internal state + 1 copy to final subscriber
                                                           .move_count = 0},
                                          .send_by_move = {.copy_count = 1, // 1 copy to internal state
                                                           .move_count = 1} // 1 move to final subscriber
                                      });
}