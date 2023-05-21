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

#include <rpp/operators/scan.hpp>
#include <rpp/sources/just.hpp>

#include "mock_observer.hpp"

TEST_CASE("scan scans values and store state")
{
    auto obs = rpp::source::just(1, 2, 3);

    SECTION("subscribe on it via scan with plus")
    {
        auto mock = mock_observer_strategy<int>{};
        
        obs | rpp::operators::scan(10, std::plus<int>{}) | rpp::operators::subscribe(mock.get_observer());
        SECTION("observer obtains partial sums")
        {
            CHECK(mock.get_received_values() == std::vector{10, 11, 13, 16});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe on it via scan with plus with no seed")
    {
        auto mock = mock_observer_strategy<int>{};

        obs | rpp::operators::scan(std::plus<int>{}) | rpp::operators::subscribe(mock.get_observer());
        SECTION("observer obtains partial sums")
        {
            CHECK(mock.get_received_values() == std::vector{1, 3, 6});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe on it via scan as lvalue with plus")
    {
        auto mock = mock_observer_strategy<int>{};
        
        auto op =  rpp::operators::scan(10, std::plus<int>{});
        obs | op | rpp::operators::subscribe(mock.get_observer());
        SECTION("observer obtains partial sums")
        {
            CHECK(mock.get_received_values() == std::vector{10, 11, 13, 16});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe on it via scan with plus as lvalue with no seed")
    {
        auto mock = mock_observer_strategy<int>{};

        auto op = rpp::operators::scan(std::plus<int>{});
        obs | op | rpp::operators::subscribe(mock.get_observer());
        SECTION("observer obtains partial sums")
        {
            CHECK(mock.get_received_values() == std::vector{1, 3, 6});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe on it via scan with aggregating in vector")
    {
        auto mock = mock_observer_strategy<std::vector<int>>{};

        obs
            | rpp::operators::scan(std::vector<int>{},
                    [](std::vector<int>&& seed, int new_val)
                    {
                        seed.push_back(new_val);
                        return std::move(seed);
                    })
            | rpp::operators::subscribe(mock.get_observer());

        SECTION("observer obtains partial vectors")
        {
            CHECK(mock.get_received_values() == std::vector{std::vector<int>{},
                                                            std::vector{1},
                                                            std::vector{1,2},
                                                            std::vector{1,2,3}});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
        }
    }
    SECTION("subscribe on it via scan with exception")
    {
        auto mock = mock_observer_strategy<int>{};

        volatile bool none{};
        obs | rpp::operators::scan(0,
                    [&](int, int)-> int
                    {
                        if (none)
                            return 0;
                        throw std::runtime_error{""};
                    })
            | rpp::operators::subscribe(mock.get_observer());

        SECTION("observer obtains only on_error")
        {
            CHECK(mock.get_received_values() == std::vector{0});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
    SECTION("subscribe on it via scan with exception with no seed")
    {
        auto mock = mock_observer_strategy<int>{};

        volatile bool none{};
        obs | rpp::operators::scan([&](int, int)-> int
                    {
                        if (none)
                            return 0;
                        throw std::runtime_error{""};
                    })
            | rpp::operators::subscribe(mock.get_observer());

        SECTION("observer obtains only on_error")
        {
            CHECK(mock.get_received_values() == std::vector{1});
            CHECK(mock.get_on_error_count() == 1);
            CHECK(mock.get_on_completed_count() == 0);
        }
    }
}