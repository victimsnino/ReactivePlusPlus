//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "mock_observer.hpp"

#include <catch2/catch_test_macros.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/operators/take.hpp>

#include <rpp/operators/group_by.hpp>

SCENARIO("group_by emits grouped seqences of values", "[group_by]")
{
    auto obs = mock_observer<rpp::grouped_observable_group_by<int, int>>{};
    std::map<int, mock_observer<int>> grouped_mocks{};

    GIVEN("observable of values")
    {
        auto observable = rpp::source::just(1, 2, 3, 4, 4, 3, 2, 1);
        WHEN("subscribe on it via group_by with identity")
        {
            auto gr_by_obs = observable.group_by(std::identity{});
            THEN("Obtained same amount of grouped observables as amount of unique values")
            {
                gr_by_obs.subscribe(obs);
                CHECK(obs.get_total_on_next_count() == 4);
                CHECK(obs.get_on_error_count() == 0);
                CHECK(obs.get_on_completed_count() == 1);
            }
            THEN("each grouped observable emits only same values")
            {
                gr_by_obs.subscribe([&](const auto& grouped)
                {
                    REQUIRE(grouped_mocks.contains(grouped.get_key()) == false);
                    grouped.subscribe(grouped_mocks[grouped.get_key()]);
                });

                CHECK(grouped_mocks.size() == 4);
                for(const auto& [key, observer] : grouped_mocks)
                {
                    CHECK(std::ranges::all_of(observer.get_received_values(), [&](int v){return v == key;}));
                    CHECK(observer.get_total_on_next_count() == 2);
                    CHECK(observer.get_on_error_count() == 0);
                    CHECK(observer.get_on_completed_count() == 1);
                }
            }
            AND_WHEN("grouped observables with key 4 unsubscribed early")
            {
                gr_by_obs.subscribe([&](const auto& grouped)
                {
                    auto key = grouped.get_key();
                    REQUIRE(grouped_mocks.contains(key) == false);

                    if (key == 4)
                        grouped.take(1).subscribe(grouped_mocks[key]);
                    else
                        grouped.subscribe(grouped_mocks[key]);
                });

                THEN("all except of key 4 obtains as before, but key 4 obtained once")
                {
                    CHECK(grouped_mocks.size() == 4);
                    for(const auto& [key, observer] : grouped_mocks)
                    {
                        CHECK(std::ranges::all_of(observer.get_received_values(), [&](int v){return v == key;}));
                        if (key == 4)
                            CHECK(observer.get_total_on_next_count() == 1);
                        else
                            CHECK(observer.get_total_on_next_count() == 2);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 1);
                    }
                }
            }
            AND_WHEN("subscribe only on one grouped observable and unsubcribe from root")
            {
                gr_by_obs.take(1).subscribe([&](const auto& grouped)
                {
                     grouped.subscribe(grouped_mocks[grouped.get_key()]);
                });
                THEN("values for such a observable are still obtainable")
                {
                    CHECK(grouped_mocks.size() == 1);

                    for(const auto& [key, observer] : grouped_mocks)
                    {
                        CHECK(std::ranges::all_of(observer.get_received_values(), [&](int v){return v == key;}));
                        CHECK(observer.get_total_on_next_count() == 2);
                        CHECK(observer.get_on_error_count() == 0);
                        CHECK(observer.get_on_completed_count() == 1);
                    }
                }
            }
        }
    }
}

SCENARIO("group_by keeps subscription till anyone subscribed", "[group_by]")
{
    GIVEN("subject of values")
    {
        auto obs = rpp::subjects::publish_subject<int>{};
        WHEN("subscribe on it via group_by")
        {
            auto                                     grouped = obs.get_observable().group_by(std::identity{});
            std::vector<rpp::composite_subscription> sub_subscriptions{};
            size_t on_error_count = 0;
            size_t on_completed_count = 0;
            auto on_error = [&](std::exception_ptr){++on_error_count;};
            auto on_completed = [&](){++on_completed_count;};
            auto                                     sub = grouped.subscribe([&](const auto& observable)
            {
                sub_subscriptions.push_back(observable.subscribe([](auto){}, on_error, on_completed));
            }, on_error, on_completed);

            THEN("subscription is alive")
            {
                CHECK(sub.is_subscribed());
            }
            AND_WHEN("send values")
            {
                obs.get_subscriber().on_next(1);
                obs.get_subscriber().on_next(2);
                THEN("subscription and sub-subscriptions are alive")
                {
                    CHECK(sub.is_subscribed());
                    CHECK(sub_subscriptions.size() == 2);
                    CHECK(std::ranges::all_of(sub_subscriptions, [](const auto& sub){return sub.is_subscribed();}));
                    AND_WHEN("unsubscribe root")
                    {
                        sub.unsubscribe();
                        THEN("sub-subscriptions are still alive")
                        {
                            CHECK(std::ranges::all_of(sub_subscriptions, [](const auto& sub){return sub.is_subscribed();}));
                        }
                    }
                    AND_WHEN("unsubscribe sub-subscriptions")
                    {
                        std::ranges::for_each(sub_subscriptions, &rpp::composite_subscription::unsubscribe);
                        THEN("root subscription is still alive")
                        {
                            CHECK(sub.is_subscribed());
                        }
                    }
                    AND_WHEN("unsubscribe all")
                    {
                        sub.unsubscribe();
                        std::ranges::for_each(sub_subscriptions, &rpp::composite_subscription::unsubscribe);
                        THEN("no any active subscriptions")
                        {
                            CHECK(!sub.is_subscribed());
                            CHECK(std::ranges::all_of(sub_subscriptions, [](const auto& sub){return !sub.is_subscribed();}));
                        }
                    }
                    AND_WHEN("send on_error")
                    {
                        obs.get_subscriber().on_error(std::make_exception_ptr(std::runtime_error{""}));
                        THEN("no any active subscriptions")
                        {
                            CHECK(!sub.is_subscribed());
                            CHECK(std::ranges::all_of(sub_subscriptions, [](const auto& sub){return !sub.is_subscribed();}));
                            CHECK(on_error_count == sub_subscriptions.size()+1);
                        }
                    }
                     AND_WHEN("send on_completed")
                    {
                        obs.get_subscriber().on_completed();
                        THEN("no any active subscriptions")
                        {
                            CHECK(!sub.is_subscribed());
                            CHECK(std::ranges::all_of(sub_subscriptions, [](const auto& sub){return !sub.is_subscribed();}));
                            CHECK(on_completed_count == sub_subscriptions.size()+1);

                        }
                    }
                }
            }
        }
    }
}

SCENARIO("group_by selectors affects types", "[group_by]")
{
    GIVEN("observable")
    {
        auto obs = rpp::observable::just(1,2,3,1,2,3);
        WHEN("subscribe on it via group_by with const key selector")
        {
            std::vector<int> keys{};
            obs.group_by([](int){return 1;}).subscribe([&](const auto& grouped)
            {
                keys.push_back(grouped.get_key());
            });

            THEN("only one unique key obtained")
            {
                CHECK(keys == std::vector{1});
            }
        }
        WHEN("subscribe on it via group_by with identity key selector")
        {
            std::vector<int> keys{};
            obs.group_by(std::identity{}).subscribe([&](const auto& grouped)
            {
                keys.push_back(grouped.get_key());
            });

            THEN("all values obtained as keys")
            {
                CHECK(keys == std::vector{1,2,3});
            }
        }
        WHEN("subscribe on it via group_by with value selector")
        {
            auto mock = mock_observer<std::string>{};
            obs.group_by(std::identity{}, [](int v){return std::to_string(v);}).subscribe([&](const auto& grouped)
            {
                grouped.subscribe(mock);
            });

            THEN("grouped observables provides modified values")
            {
                using namespace std::string_literals;

                CHECK(mock.get_received_values() == std::vector{"1"s, "2"s, "3"s, "1"s, "2"s, "3"s});
            }
        }
        WHEN("subscribe on it via group_by with comparator as all similar")
        {
            std::vector<int> keys{};
            obs.group_by(std::identity{}, std::identity{}, [](int, int){return false;}).subscribe([&](const auto& grouped)
            {
                keys.push_back(grouped.get_key());
            });

            THEN("comparator interpets keys as similar")
            {
                CHECK(keys == std::vector{1});
            }
        }
        WHEN("subscribe on it via group_by with comparator as all different")
        {
            std::vector<int> keys{};
            obs.group_by(std::identity{}, std::identity{}, [](int, int){return true;}).subscribe([&](const auto& grouped)
            {
                keys.push_back(grouped.get_key());
            });

            THEN("comparator interpets keys as different")
            {
                CHECK(keys == std::vector{1,2,3,1,2,3});
            }
        }
        auto mock = mock_observer<rpp::grouped_observable_group_by<int, int>>{};

        WHEN("subscribe on it via group_by with key selector with exception")
        {
            obs.group_by([](int) -> int {throw std::runtime_error{""};}).subscribe(mock);
            THEN("on_error obtained once")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }

        WHEN("subscribe on it via group_by with value selector with exception")
        {
            obs.group_by(std::identity{}, [](int) -> int {throw std::runtime_error{""};}).subscribe(mock);
            THEN("on_error obtained once")
            {
                CHECK(mock.get_total_on_next_count() == 1);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}