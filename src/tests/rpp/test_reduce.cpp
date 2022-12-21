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

#include <rpp/operators/reduce.hpp>

#include <rpp/sources/just.hpp>
#include <rpp/sources/empty.hpp>

SCENARIO("reduce reduces values and store state", "[reduce]")
{
    GIVEN("observable")
    {
        auto obs = rpp::observable::just(1, 2, 3);
        WHEN("subscribe on it via reduce with plus")
        {
            auto mock = mock_observer<int>{};

            obs.reduce(0, std::plus<int>{}).subscribe(mock);
            THEN("observer obtains final sum")
            {
                CHECK(mock.get_received_values() == std::vector{6});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via reduce with aggregating in vector")
        {
            auto mock = mock_observer<std::vector<int>>{};

            obs.reduce(std::vector<int>{},
                     [](std::vector<int>&& seed, int new_val)
                     {
                         seed.push_back(new_val);
                         return std::move(seed);
                     }).subscribe(mock);

            THEN("observer obtains final vector")
            {
                CHECK(mock.get_received_values() == std::vector{{std::vector{1,2,3}}});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via reduce with exception")
        {
            auto mock = mock_observer<int>{};

            volatile bool none{};
            obs.reduce(0,
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
    GIVEN("observable 1-2->")
    {
        auto obs = rpp::observable::create<int>([](const auto& sub)
        {
            sub.on_next(1);
            sub.on_next(2);
        });
        WHEN("subscribe on it via reduce with plus")
        {
            auto mock = mock_observer<int>{};

            obs.reduce(0, std::plus<int>{}).subscribe(mock);
            THEN("no any values")
            {
                CHECK(mock.get_received_values() == std::vector<int>{});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("average calculates average", "[reduce]")
{
    GIVEN("-1-2| observable")
    {
        auto obs = rpp::source::just(1,2);
        WHEN("subscribe on it via average")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.average().subscribe(mock);

            THEN("observer obtained value as average int")
            {
                CHECK(mock.get_received_values() == std::vector{static_cast<int>(1+2)/2});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
        WHEN("subscribe on it via average<double>")
        {
            auto mock = mock_observer<double>{};
            obs.average<double>().subscribe(mock);

            THEN("observer obtained value as average double")
            {
                CHECK(mock.get_received_values() == std::vector<double>{1.5});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("-| observable")
    {
        auto obs = rpp::source::empty<int>();
        WHEN("subscribe on it via average")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.average().subscribe(mock);

            THEN("observer obtained on_error")
            {
                CHECK(mock.get_received_values() == std::vector<int>{});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("sum calculates sum", "[reduce]")
{
    GIVEN("-1-2| observable")
    {
        auto obs = rpp::source::just(1,2);
        WHEN("subscribe on it via sum")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.sum().subscribe(mock);

            THEN("observer obtained value as sum int")
            {
                CHECK(mock.get_received_values() == std::vector{3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("-| observable")
    {
        auto obs = rpp::source::empty<int>();
        WHEN("subscribe on it via sum")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.sum().subscribe(mock);

            THEN("observer obtained on_error")
            {
                CHECK(mock.get_received_values() == std::vector<int>{});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("count calculates count", "[reduce]")
{
    GIVEN("-1-2| observable")
    {
        auto obs = rpp::source::just(1,2);
        WHEN("subscribe on it via count")
        {
            auto mock = mock_observer<size_t>{};
            auto r= obs.count().subscribe(mock);

            THEN("observer obtained value as count size_t")
            {
                CHECK(mock.get_received_values() == std::vector{size_t{2}});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("-| observable")
    {
        auto obs = rpp::source::empty<int>();
        WHEN("subscribe on it via count")
        {
            auto mock = mock_observer<size_t>{};
            auto r= obs.count().subscribe(mock);

            THEN("observer obtained zero")
            {
                CHECK(mock.get_received_values() == std::vector<size_t>{0});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("min calculates min", "[reduce]")
{
    GIVEN("-3-1-2| observable")
    {
        auto obs = rpp::source::just(3,1,2);
        WHEN("subscribe on it via min")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.min().subscribe(mock);

            THEN("observer obtained min value")
            {
                CHECK(mock.get_received_values() == std::vector{1});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("-| observable")
    {
        auto obs = rpp::source::empty<int>();
        WHEN("subscribe on it via min")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.min().subscribe(mock);

            THEN("observer obtained on_error")
            {
                CHECK(mock.get_received_values() == std::vector<int>{});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("max calculates min", "[reduce]")
{
    GIVEN("-3-1-2| observable")
    {
        auto obs = rpp::source::just(3,1,2);
        WHEN("subscribe on it via max")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.max().subscribe(mock);

            THEN("observer obtained max value")
            {
                CHECK(mock.get_received_values() == std::vector{3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    GIVEN("-| observable")
    {
        auto obs = rpp::source::empty<int>();
        WHEN("subscribe on it via max")
        {
            auto mock = mock_observer<int>{};
            auto r= obs.max().subscribe(mock);

            THEN("observer obtained on_error")
            {
                CHECK(mock.get_received_values() == std::vector<int>{});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

SCENARIO("reduce keeps state for copies", "[reduce]")
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
            sub.on_completed();
        });
        WHEN("subscribe on it via reduce")
        {
            obs.reduce(int{}, [](int seed, int new_v) { return seed + new_v; }).subscribe(mock);
            THEN("observer obtains only last value")
            {
                CHECK(mock.get_received_values() == std::vector{ 10 });
            }
        }
    }
}

SCENARIO("reduce doesn't produce extra copies", "[reduce][track_copy]")
{
    GIVEN("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto               obs = rpp::source::just(1).reduce(verifier, [](copy_count_tracker&& seed, int) { return std::move(seed); });
        WHEN("subscribe")
        {
            obs.subscribe([](const auto&) {});
            THEN("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 2); // 1 copy to reduce state + 1 copy for provided subscriber to shared_state
                REQUIRE(verifier.get_move_count() == 5);
                // 1 move to observable state + 1 move to subscriber + 1 move from lambda + 1 move to new_state + 1 move to final lambda
            }
        }
    }
}
