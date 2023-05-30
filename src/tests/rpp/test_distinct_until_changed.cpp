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

TEST_CASE("distinct_until_changed filters out consecutive duplicates and send first value from duplicates")
{
    auto mock = mock_observer_strategy<int>{};
    SECTION("observable of values with duplicates")
    {
        auto obs = rpp::source::just(1, 1, 2, 2, 3, 2, 2, 1);
        SECTION("when subscribe on it via distinct_until_changed then subscriber obtains values without consecutive duplicates")
        {
            obs | rpp::ops::distinct_until_changed() | rpp::ops::subscribe(mock.get_observer());
            CHECK(mock.get_received_values() == std::vector{ 1,2,3,2,1 });
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
     }
        SECTION("when subscribe on it via distinct_until_changed with custom comparator then subscriber obtains values without consecutive duplicates")
        {
            obs | rpp::ops::distinct_until_changed([](int old_value, int new_value) {return old_value % 2 != new_value % 2; }) | rpp::ops::subscribe(mock.get_observer());
            CHECK(mock.get_received_values() == std::vector{ 1, 1, 3, 1});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }

    // SECTION("subject of values")
    // {
    //     auto subj = rpp::subjects::publish_subject<int>{};
    //     SECTION("subscribe on it via distinct_until_changed and send value")
    //     {
    //         subj.get_observable().distinct_until_changed().subscribe(mock);
    //         subj.get_subscriber().on_next(1);
    //         SECTION("subscriber obtains value")
    //         {
    //             CHECK(mock.get_received_values() == std::vector{ 1});
    //             CHECK(mock.get_on_error_count() == 0);
    //             CHECK(mock.get_on_completed_count() == 0);
    //         }
    //         AND_SECTION("send duplicate")
    //         {
    //             subj.get_subscriber().on_next(1);
    //             SECTION("subscriber ignores duplicated value")
    //             {
    //                 CHECK(mock.get_received_values() == std::vector{ 1 });
    //                 CHECK(mock.get_on_error_count() == 0);
    //                 CHECK(mock.get_on_completed_count() == 0);
    //             }
    //         }
    //     }
    // }
}