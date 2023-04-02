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

#include <rpp/sources/from.hpp>
#include <functional>
#include "mock_observer.hpp"
#include "copy_count_tracker.hpp"

TEST_CASE("from iterable emit items from container")
{
    auto mock = mock_observer_strategy<int>();
    SECTION("observable created from vector emits items to observer in the same order")
    {
        auto vals = std::vector{1, 2, 3, 4, 5, 6};
        auto obs  = rpp::source::from_iterable(vals);
        obs.subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == vals);
        CHECK(mock.get_on_completed_count() == 1);
    }
    // SECTION("observable from iterable with scheduler")
    // {
    //     auto vals     = std::vector{1, 2, 3, 4, 5, 6};
    //     auto run_loop = rpp::schedulers::run_loop{};
    //     auto obs      = rpp::source::from_iterable(vals, run_loop);
    //     SECTION("subscribe on it and dispatch once")
    //     {
    //         obs.subscribe(mock);
    //         run_loop.dispatch();
    //         SECTION("observer obtains first value")
    //         {
    //             CHECK(mock.get_received_values() == std::vector{1});
    //             CHECK(mock.get_on_error_count() == 0);
    //             CHECK(mock.get_on_completed_count() == 0);
    //         }
    //         AND_SECTION("dispatch till no events")
    //         {
    //             while (!run_loop.is_empty())
    //                 run_loop.dispatch();

    //             SECTION("observer obtains values in the same order")
    //             {
    //                 CHECK(mock.get_received_values() == vals);
    //                 CHECK(mock.get_on_completed_count() == 1);
    //             }
    //         }
    //     }
    //     SECTION("subscribe on it, unsubscribe on first on next and dispatch till not empty")
    //     {
    //         rpp::composite_subscription sub{};
    //         obs.subscribe(sub,
    //                       [&](const auto& v)
    //                       {
    //                           mock.on_next(v);
    //                           sub.unsubscribe();
    //                       });

    //         size_t dispatch_count{};
    //         while (!run_loop.is_empty())
    //         {
    //             ++dispatch_count;
    //             run_loop.dispatch();
    //         }

    //         SECTION("observer obtains first value")
    //         {
    //             CHECK(mock.get_received_values() == std::vector{ 1 });
    //             CHECK(mock.get_on_error_count() == 0);
    //             CHECK(mock.get_on_completed_count() == 0);
    //             CHECK(sub.is_subscribed() == false);
    //             CHECK(dispatch_count == 1);
    //         }
    //     }
    // }
    // SECTION("observable from iterable with exceiption on begin")
    // {
    //     auto run_loop = rpp::schedulers::run_loop{};
    //     auto obs = rpp::source::from_iterable(my_container_with_error{}, run_loop);
    //     SECTION("subscribe on it and dispatch once")
    //     {
    //         obs.subscribe(mock);
    //         run_loop.dispatch();
    //         SECTION("observer obtains error")
    //         {
    //             CHECK(mock.get_total_on_next_count() == 0);
    //             CHECK(mock.get_on_error_count() == 1);
    //             CHECK(mock.get_on_completed_count() == 0);
    //         }
    //     }
    // }
}

TEST_CASE("from iterable doesn't provides extra copies")
{
    copy_count_tracker tracker{};
    auto               vals         = std::array{tracker};
    auto               initial_copy = tracker.get_copy_count();
    auto               initial_move = tracker.get_move_count();

    SECTION("observable from copied iterable doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable(vals/*, rpp::schedulers::immediate{}*/);
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to wrapped container
        CHECK(tracker.get_move_count() - initial_move == 1); // 1 move lambda to observable
    }
    SECTION("observable from moved iterable doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable(std::move(vals)/*, rpp::schedulers::immediate{}*/);
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 0); 
        CHECK(tracker.get_move_count() - initial_move == 2); // 1 move to wrapped container + 1 move lambda to observable
    }

    SECTION("observable from copied iterable with shared memory model doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable<rpp::memory_model::use_shared>(vals);
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to shared_ptr
        CHECK(tracker.get_move_count() - initial_move == 0);
    }
    SECTION("observable from moved iterable doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable<rpp::memory_model::use_shared>(std::move(vals)); // NOLINT
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 0);
        CHECK(tracker.get_move_count() - initial_move == 1); // 1 move to shared_ptr
    }
}

// TEST_CASE("from callable", "[source][from]")
// {
//     SECTION("observable from callable")
//     {
//         size_t count_of_calls{};
//         auto callable = [&count_of_calls]() -> int {return static_cast<int>(++count_of_calls); };
//         auto observable = rpp::source::from_callable(callable);
//         SECTION("subscribe on this observable")
//         {
//             auto mock = mock_observer<int>{};
//             observable.subscribe(mock);
//             SECTION("callable called only once and observable returns value of this function")
//             {
//                 CHECK(mock.get_received_values() == std::vector{ 1 });
//                 CHECK(mock.get_on_completed_count() == 1);
//                 CHECK(count_of_calls == 1);
//             }
//         }
//     }
//     SECTION("observable from callable with error")
//     {
//         volatile bool none{true};
//         auto callable = [&]() -> int { if (none) throw std::runtime_error{ "" }; return 0; };
//         auto observable = rpp::source::from_callable(callable);
//         SECTION("subscribe on this observable")
//         {
//             auto mock = mock_observer<int>{};
//             observable.subscribe(mock);
//             SECTION("observer obtains error")
//             {
//                 CHECK(mock.get_total_on_next_count() == 0);
//                 CHECK(mock.get_on_error_count() == 1);
//                 CHECK(mock.get_on_completed_count() == 0);
//             }
//         }
//     }
// }

// TEST_CASE("just")
// {
//     mock_observer<copy_count_tracker> mock{ false };

//     SECTION("observable with copied item")
//     {
//         copy_count_tracker v{};
//         auto               obs = rpp::observable::just(rpp::schedulers::immediate{}, v);
//         SECTION("subscribe on this observable")
//         {
//             obs.subscribe(mock);
//             SECTION("value obtained")
//             {
//                 CHECK(mock.get_on_next_const_ref_count() == 1);
//                 CHECK(mock.get_on_next_move_count() == 0);
//                 CHECK(mock.get_on_completed_count() == 1);
//                 CHECK(v.get_copy_count() == 1); // 1 copy into array
//                 CHECK(v.get_move_count() <= 2); // 1 move of array into lambda + 1 move lambda into observable
//             }
//         }
//     }
//     SECTION("observable with moved item")
//     {
//         copy_count_tracker v{};
//         auto               obs = rpp::observable::just(rpp::schedulers::immediate{}, std::move(v));
//         SECTION("subscribe on this observable")
//         {
//             obs.subscribe(mock);
//             SECTION("value obtained")
//             {
//                 CHECK(mock.get_on_next_const_ref_count() == 1);
//                 CHECK(mock.get_on_next_move_count() == 0);
//                 CHECK(mock.get_on_completed_count() == 1);
//                 CHECK(v.get_copy_count() == 0);
//                 CHECK(v.get_move_count() <= 3); // 1 move into array + 1 move array to function for observable + 1 move into observable
//             }
//         }
//     }
//     SECTION("observable with copied item + use_sahred")
//     {
//         copy_count_tracker v{};
//         auto               obs = rpp::observable::just<rpp::memory_model::use_shared>(v);
//         SECTION("subscribe on this observable")
//         {
//             obs.subscribe(mock);
//             SECTION("value obtained")
//             {
//                 CHECK(mock.get_on_next_const_ref_count() == 1);
//                 CHECK(mock.get_on_next_move_count() == 0);
//                 CHECK(mock.get_on_completed_count() == 1);
//                 CHECK(v.get_copy_count() == 1); // 1 copy into shared_ptr
//                 CHECK(v.get_move_count() == 0);
//             }
//         }
//     }
//     SECTION("observable with moved item + use_shared")
//     {
//         copy_count_tracker v{};
//         auto               obs = rpp::observable::just<rpp::memory_model::use_shared>(std::move(v));
//         SECTION("subscribe on this observable")
//         {
//             obs.subscribe(mock);
//             SECTION("value obtained")
//             {
//                 CHECK(mock.get_on_next_const_ref_count() == 1);
//                 CHECK(mock.get_on_next_move_count() == 0);
//                 CHECK(mock.get_on_completed_count() == 1);
//                 CHECK(v.get_copy_count() == 0);
//                 CHECK(v.get_move_count() == 1); // 1 move into shared_ptr
//             }
//         }
//     }
// }

// TEST_CASE("just variadic")
// {
//     auto mock = mock_observer<int>();
//     SECTION("observable just variadic")
//     {
//         auto obs = rpp::source::just(1, 2, 3, 4, 5, 6);
//         SECTION("subscribe on it")
//         {
//             obs.subscribe(mock);
//             SECTION("observer obtains values in the same order")
//             {
//                 CHECK(mock.get_received_values() == std::vector{ 1, 2, 3, 4, 5, 6 });
//                 CHECK(mock.get_on_completed_count() == 1);
//             }
//         }
//     }
// }