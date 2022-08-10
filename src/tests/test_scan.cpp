//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "copy_count_tracker.hpp"
#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <rpp/operators/scan.hpp>

#include <rpp/sources/just.hpp>

SCENARIO("scan scans values and store state", "[scan]")
{
    GIVEN("observable")
    {
        auto obs = rpp::observable::just(1, 2, 3);
        WHEN("subscribe on it via scan with plus")
        {
            auto mock = mock_observer<int>{};

            obs.scan(0, std::plus<int>{}).subscribe(mock);
            THEN("observer obtains partial sums")
            {
                CHECK(mock.get_received_values() == std::vector{1, 3, 6});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via scan with aggregating in vector")
        {
            auto mock = mock_observer<std::vector<int>>{};

            obs.scan(std::vector<int>{},
                     [](std::vector<int>&& seed, int new_val)
                     {
                         seed.push_back(new_val);
                         return std::move(seed);
                     }).subscribe(mock);

            THEN("observer obtains partial vectors")
            {
                CHECK(mock.get_received_values() == std::vector{std::vector{1},
                                                                std::vector{1,2},
                                                                std::vector{1,2,3}});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via scan with exception")
        {
            auto mock = mock_observer<int>{};

            volatile bool none{};
            obs.scan(0,
                     [&](int, int)-> int
                     {
                         if (none)
                             return 0;
                         throw std::runtime_error{""};
                     }).subscribe(mock);

            THEN("observer obtains only on_error")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("scan keeps state for copies", "[scan]")
{
    auto mock = mock_observer<int>{};
    GIVEN("observable which sends values via copy")
    {
        auto obs = rpp::source::create<int>([](const auto& sub)
            {
                for (size_t i = 0; i < 10; ++i)
                {
                    auto copy = sub;
                    copy.on_next(1);
                }
            });
        WHEN("subscribe on it via scan")
        {
            obs.scan(int{}, [](int seed, int new_v) {return seed + new_v; }).subscribe(mock);
            THEN("observer obtains values as expected")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 });
            }
        }
    }
}

SCENARIO("scan doesn't produce extra copies", "[scan][track_copy]")
{
    GIVEN("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto          obs = rpp::source::just(1).scan(verifier, [](copy_count_tracker&& seed, int) { return std::move(seed); });
        WHEN("subscribe")
        {
            obs.subscribe([](const auto&){});
            THEN("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 2); // 1 copy to scan state + 1 copy for provided subscriber to shared_state
                REQUIRE(verifier.get_move_count() == 3); // 1 move to observable state + 1 move to subscriber  + 1 move from lambda
            }
        }
    }
}