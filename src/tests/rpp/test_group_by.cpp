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

#include <rpp/operators/take.hpp>
#include <rpp/operators/group_by.hpp>
#include <rpp/sources/just.hpp>

#include "mock_observer.hpp"

TEST_CASE("group_by emits grouped seqences of values with identity key selector", "[group_by]")
{
    auto obs = mock_observer_strategy<rpp::grouped_observable_group_by<int, int>>{};
    std::map<int, mock_observer_strategy<int>> grouped_mocks{};
    auto observable = rpp::source::just(1, 2, 3, 4, 4, 3, 2, 1) | rpp::operators::group_by(std::identity{});

    SECTION("Obtained same amount of grouped observables as amount of unique values")
    {
        observable.subscribe(obs.get_observer());
        REQUIRE(obs.get_total_on_next_count() == 4);
        REQUIRE(obs.get_on_error_count() == 0);
        REQUIRE(obs.get_on_completed_count() == 1);
    }
    SECTION("each grouped observable emits only same values")
    {
        observable.subscribe([&](const auto& grouped)
        {
            REQUIRE(grouped_mocks.contains(grouped.get_key()) == false);
            grouped.subscribe(grouped_mocks[grouped.get_key()].get_observer());
        });

        REQUIRE(grouped_mocks.size() == 4);
        for(const auto& [key, observer] : grouped_mocks)
        {
            REQUIRE(rpp::utils::all_of(observer.get_received_values(), [key=key](int v){return v == key;}));
            REQUIRE(observer.get_total_on_next_count() == 2);
            REQUIRE(observer.get_on_error_count() == 0);
            REQUIRE(observer.get_on_completed_count() == 1);
        }
    }
    SECTION("grouped observables with key 4 unsubscribed early")
    {
        observable.subscribe([&](const auto& grouped)
        {
            auto key = grouped.get_key();
            REQUIRE(grouped_mocks.contains(key) == false);

            if (key == 4)
                grouped | rpp::operators::take(1) | rpp::operators::subscribe(grouped_mocks[key].get_observer());
            else
                grouped.subscribe(grouped_mocks[key].get_observer());
        });

        SECTION("all except of key 4 obtains as before, but key 4 obtained once")
        {
            REQUIRE(grouped_mocks.size() == 4);
            for(const auto& [key, observer] : grouped_mocks)
            {
                REQUIRE(rpp::utils::all_of(observer.get_received_values(), [key=key](int v){return v == key;}));
                if (key == 4)
                    REQUIRE(observer.get_total_on_next_count() == 1);
                else
                    REQUIRE(observer.get_total_on_next_count() == 2);
                REQUIRE(observer.get_on_error_count() == 0);
                REQUIRE(observer.get_on_completed_count() == 1);
            }
        }
    }
    SECTION("subscribe only on one grouped observable and unsubcribe from root")
    {
        observable | rpp::ops::take(1) | rpp::ops::subscribe([&](const auto& grouped)
        {
            grouped.subscribe(grouped_mocks[grouped.get_key()].get_observer());
        });
        SECTION("values for such a observable are still obtainable")
        {
            REQUIRE(grouped_mocks.size() == 1);

            for(const auto& [key, observer] : grouped_mocks)
            {
                REQUIRE(rpp::utils::all_of(observer.get_received_values(), [key=key](int v){return v == key;}));
                REQUIRE(observer.get_total_on_next_count() == 2);
                REQUIRE(observer.get_on_error_count() == 0);
                REQUIRE(observer.get_on_completed_count() == 1);
            }
        }
    }
}

// TEST_CASE("group_by keeps subscription till anyone subscribed", "[group_by]")
// {
//     SECTION("subject of values")
//     {
//         auto obs = rpp::subjects::publish_subject<int>{};
//         SECTION("subscribe on it via group_by")
//         {
//             auto                                     grouped = obs.get_observable().group_by(std::identity{});
//             std::vector<rpp::composite_subscription> sub_subscriptions{};
//             size_t on_error_count = 0;
//             size_t on_completed_count = 0;
//             auto on_error = [&](std::exception_ptr){++on_error_count;};
//             auto on_completed = [&](){++on_completed_count;};
//             auto                                     sub = grouped.subscribe([&](const auto& observable)
//             {
//                 sub_subscriptions.push_back(observable.subscribe([](auto){}, on_error, on_completed));
//             }, on_error, on_completed);

//             SECTION("subscription is alive")
//             {
//                 REQUIRE(sub.is_subscribed());
//             }
//             AND_SECTION("send values")
//             {
//                 obs.get_subscriber().on_next(1);
//                 obs.get_subscriber().on_next(2);
//                 SECTION("subscription and sub-subscriptions are alive")
//                 {
//                     REQUIRE(sub.is_subscribed());
//                     REQUIRE(sub_subscriptions.size() == 2);
//                     REQUIRE(rpp::utils::all_of(sub_subscriptions, [](const auto& sub){return sub.is_subscribed();}));
//                     AND_SECTION("unsubscribe root")
//                     {
//                         sub.unsubscribe();
//                         SECTION("sub-subscriptions are still alive")
//                         {
//                             REQUIRE(rpp::utils::all_of(sub_subscriptions, [](const auto& sub){return sub.is_subscribed();}));
//                         }
//                     }
//                     AND_SECTION("unsubscribe sub-subscriptions")
//                     {
//                         rpp::utils::for_each(sub_subscriptions, std::mem_fn(&rpp::composite_subscription::unsubscribe));
//                         SECTION("root subscription is still alive")
//                         {
//                             REQUIRE(sub.is_subscribed());
//                         }
//                     }
//                     AND_SECTION("unsubscribe all")
//                     {
//                         sub.unsubscribe();
//                         rpp::utils::for_each(sub_subscriptions, std::mem_fn(&rpp::composite_subscription::unsubscribe));
//                         SECTION("no any active subscriptions")
//                         {
//                             REQUIRE(!sub.is_subscribed());
//                             REQUIRE(rpp::utils::all_of(sub_subscriptions, [](const auto& sub){return !sub.is_subscribed();}));
//                         }
//                     }
//                     AND_SECTION("send on_error")
//                     {
//                         obs.get_subscriber().on_error(std::make_exception_ptr(std::runtime_error{""}));
//                         SECTION("no any active subscriptions")
//                         {
//                             REQUIRE(!sub.is_subscribed());
//                             REQUIRE(rpp::utils::all_of(sub_subscriptions, [](const auto& sub){return !sub.is_subscribed();}));
//                             REQUIRE(on_error_count == sub_subscriptions.size()+1);
//                         }
//                     }
//                      AND_SECTION("send on_completed")
//                     {
//                         obs.get_subscriber().on_completed();
//                         SECTION("no any active subscriptions")
//                         {
//                             REQUIRE(!sub.is_subscribed());
//                             REQUIRE(rpp::utils::all_of(sub_subscriptions, [](const auto& sub){return !sub.is_subscribed();}));
//                             REQUIRE(on_completed_count == sub_subscriptions.size()+1);

//                         }
//                     }
//                 }
//             }
//         }
//     }
// }

// TEST_CASE("group_by selectors affects types", "[group_by]")
// {
//     SECTION("observable")
//     {
//         auto obs = rpp::observable::just(1,2,3,1,2,3);
//         SECTION("subscribe on it via group_by with const key selector")
//         {
//             std::vector<int> keys{};
//             obs.group_by([](int){return 1;}).subscribe([&](const auto& grouped)
//             {
//                 keys.push_back(grouped.get_key());
//             });

//             SECTION("only one unique key obtained")
//             {
//                 REQUIRE(keys == std::vector{1});
//             }
//         }
//         SECTION("subscribe on it via group_by with identity key selector")
//         {
//             std::vector<int> keys{};
//             obs.group_by(std::identity{}).subscribe([&](const auto& grouped)
//             {
//                 keys.push_back(grouped.get_key());
//             });

//             SECTION("all values obtained as keys")
//             {
//                 REQUIRE(keys == std::vector{1,2,3});
//             }
//         }
//         SECTION("subscribe on it via group_by with value selector")
//         {
//             auto mock = mock_observer_strategy<std::string>{};
//             obs.group_by(std::identity{}, [](int v){return std::to_string(v);}).subscribe([&](const auto& grouped)
//             {
//                 grouped.subscribe(mock);
//             });

//             SECTION("grouped observables provides modified values")
//             {
//                 using namespace std::string_literals;

//                 REQUIRE(mock.get_received_values() == std::vector{"1"s, "2"s, "3"s, "1"s, "2"s, "3"s});
//             }
//         }
//         SECTION("subscribe on it via group_by with custom comparator")
//         {
//             std::vector<int> keys{};
//             obs.group_by(std::identity{},
//                         std::identity{},
//                         [](int f, int s)
//                         {
//                             return f % 2 < s %2;
//                         }).subscribe([&](const auto& grouped)
//             {
//                 keys.push_back(grouped.get_key());
//             });

//             SECTION("only 2 types of keys interpreted as unique")
//             {
//                 REQUIRE(keys == std::vector{1,2});
//             }
//         }
//         auto mock = mock_observer_strategy<rpp::grouped_observable_group_by<int, int>>{};

//         SECTION("subscribe on it via group_by with key selector with exception")
//         {
//             obs.group_by([](int) -> int {throw std::runtime_error{""};}).subscribe(mock);
//             SECTION("on_error obtained once")
//             {
//                 REQUIRE(mock.get_total_on_next_count() == 0);
//                 REQUIRE(mock.get_on_error_count() == 1);
//                 REQUIRE(mock.get_on_completed_count() == 0);
//             }
//         }

//         SECTION("subscribe on it via group_by with value selector with exception")
//         {
//             obs.group_by(std::identity{}, [](int) -> int {throw std::runtime_error{""};}).subscribe(mock);
//             SECTION("on_error obtained once")
//             {
//                 REQUIRE(mock.get_total_on_next_count() == 1);
//                 REQUIRE(mock.get_on_error_count() == 1);
//                 REQUIRE(mock.get_on_completed_count() == 0);
//             }
//         }
//     }
// }