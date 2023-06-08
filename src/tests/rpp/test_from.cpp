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

#include <rpp/sources/from.hpp>
#include <rpp/operators/take.hpp>
#include <cstddef>
#include <functional>
#include "mock_observer.hpp"
#include "copy_count_tracker.hpp"
#include "rpp/memory_model.hpp"
#include "rpp/observers/fwd.hpp"
#include "rpp/schedulers/current_thread.hpp"
#include "rpp/schedulers/fwd.hpp"
#include "rpp/schedulers/immediate.hpp"
#include "rpp/sources/fwd.hpp"

#include <optional>
#include <stdexcept>

struct my_container_with_error : std::vector<int>
{
    using std::vector<int>::vector;
    std::vector<int>::const_iterator begin() const { throw std::runtime_error{"EXCEPTION ON BEGIN"}; }
};

struct infinite_container
{
    struct iterator
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = int;
        using pointer           = int*;

        value_type operator*() const { return 1; }
        iterator& operator++() { return *this; }
        iterator operator++(int) { return *this; }
        friend bool operator== (const iterator&, const iterator&) { return false; };
        friend bool operator!= (const iterator&, const iterator&) { return true; };
    };

    iterator begin() const { return {}; }
    iterator end() const { return {}; }
};


TEMPLATE_TEST_CASE("from iterable emit items from container",
                   "",
                   std::pair<rpp::schedulers::current_thread, rpp::memory_model::use_stack>,
                   std::pair<rpp::schedulers::immediate, rpp::memory_model::use_stack>,
                   std::pair<rpp::schedulers::current_thread, rpp::memory_model::use_shared>,
                   std::pair<rpp::schedulers::immediate, rpp::memory_model::use_shared>)
{
    using memory_model = std::tuple_element_t<1, TestType>;
    using scheduler = std::tuple_element_t<0, TestType>;
    auto mock = mock_observer_strategy<int>();
    SECTION("observable created from vector emits items to observer in the same order")
    {
        auto vals = std::vector{1, 2, 3, 4, 5, 6};
        auto obs  = rpp::source::from_iterable<memory_model>(vals, scheduler{});
        obs.subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == vals);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("observable created from empty vector emits only on_completed")
    {
        auto vals = std::vector<int>{};
        auto obs  = rpp::source::from_iterable<memory_model>(vals, scheduler{});
        obs.subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == vals);
        CHECK(mock.get_on_completed_count() == 1);
    }

    SECTION("subscribe via take(1) to observable created from infinite container")
    {
        rpp::source::from_iterable<memory_model>(infinite_container{}, scheduler{}) | rpp::operators::take(1)
                                                                     | rpp::operators::subscribe(mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1});
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
    SECTION("observable from iterable with exceiption on begin")
    {
        const auto obs = rpp::source::from_iterable<memory_model>(my_container_with_error{}, scheduler{});
        SECTION("subscribe on it and dispatch once")
        {
            obs.subscribe(mock.get_observer());
            SECTION("observer obtains error")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

TEMPLATE_TEST_CASE("from iterable with different schedulers", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    auto mock = mock_observer_strategy<int>();

    SECTION("observable created from vector with scheduler emits values via provided scheduler")
    {
        auto obs_immediate = rpp::source::from_iterable<TestType>(std::vector{1, 1, 1}, rpp::schedulers::immediate{});
        auto obs_default   = rpp::source::from_iterable<TestType>(std::vector{2, 2, 2});

        rpp::schedulers::current_thread::create_worker().schedule([&obs_immediate, &obs_default, &mock](const auto&)
        {
            obs_default.subscribe(mock.get_observer());
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 0);

            obs_immediate.subscribe(mock.get_observer());
            CHECK(mock.get_received_values() == std::vector{1,1,1});
            CHECK(mock.get_on_error_count() == 0);
            CHECK(mock.get_on_completed_count() == 1);
            return rpp::schedulers::optional_duration{};
        }, mock.get_observer());

        CHECK(mock.get_received_values() == std::vector{1,1,1,2,2,2});
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 2);
    }
}

TEMPLATE_TEST_CASE("from iterable doesn't provides extra copies", "", rpp::schedulers::current_thread, rpp::schedulers::immediate)
{
    copy_count_tracker tracker{};
    auto               vals         = std::array{tracker};
    auto               initial_copy = tracker.get_copy_count();
    auto               initial_move = tracker.get_move_count();

    SECTION("observable from copied iterable doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable(vals, TestType{});
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to wrapped container
        CHECK(tracker.get_move_count() - initial_move == 1); // 1 move lambda to observable
    }
    SECTION("observable from moved iterable doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable(std::move(vals), TestType{});
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 0);
        CHECK(tracker.get_move_count() - initial_move == 2); // 1 move to wrapped container + 1 move lambda to observable
    }

    SECTION("observable from copied iterable with shared memory model doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable<rpp::memory_model::use_shared>(vals, TestType{}); // NOLINT
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to shared_ptr
        CHECK(tracker.get_move_count() - initial_move == 0);
    }
    SECTION("observable from moved iterable doesn't provide extra copies")
    {
        auto obs = rpp::source::from_iterable<rpp::memory_model::use_shared>(std::move(vals), TestType{}); // NOLINT
        obs.subscribe([](const auto&){},[](const auto&){},[](){});
        CHECK(tracker.get_copy_count() - initial_copy == 0);
        CHECK(tracker.get_move_count() - initial_move == 1); // 1 move to shared_ptr
    }
}

TEST_CASE("from callable")
{
    auto mock = mock_observer_strategy<int>{};

    SECTION("observable from callable")
    {
        size_t count_of_calls{};
        auto callable = [&count_of_calls]() -> int {return static_cast<int>(++count_of_calls); };

        auto observable = rpp::source::from_callable(callable);
        SECTION("subscribe on this observable")
        {
            observable.subscribe(mock.get_observer());
            SECTION("callable called only once and observable returns value of this function")
            {
                CHECK(mock.get_received_values() == std::vector{ 1 });
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(count_of_calls == 1);
            }
        }
    }
    SECTION("observable from callable with void")
    {
        size_t count_of_calls{};
        auto callable = [&count_of_calls]() -> void { ++count_of_calls; };

        auto observable = rpp::source::from_callable(callable);
        auto none_mock = mock_observer_strategy<rpp::utils::none>{};

        SECTION("subscribe on this observable")
        {
            observable.subscribe(none_mock.get_observer());
            SECTION("callable called only once and observable returns value of this function")
            {
                CHECK(none_mock.get_received_values().size() == 1);
                CHECK(none_mock.get_on_completed_count() == 1);
                CHECK(count_of_calls == 1);
            }
        }
    }
    SECTION("observable from callable with error")
    {
        volatile bool none{true};
        auto callable = [&]() -> int { if (none) throw std::runtime_error{ "" }; return 0; };
        auto observable = rpp::source::from_callable(callable);
        SECTION("subscribe on this observable")
        {
            observable.subscribe(mock.get_observer());
            SECTION("observer obtains error")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
}

TEST_CASE("just")
{
    mock_observer_strategy<copy_count_tracker> mock{ false };

    SECTION("observable with copied item")
    {
        copy_count_tracker v{};
        auto               obs = rpp::source::just(v);
        SECTION("subscribe on this observable")
        {
            obs.subscribe(mock.get_observer());
            SECTION("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 1); // 1 copy into array
                CHECK(v.get_move_count() == 1); // 1 move lambda into observable
            }
        }
    }
    SECTION("observable with moved item")
    {
        copy_count_tracker v{};
        auto               obs = rpp::source::just(std::move(v));
        SECTION("subscribe on this observable")
        {
            obs.subscribe(mock.get_observer());
            SECTION("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 0); // NOLINT
                CHECK(v.get_move_count() == 2); // 1 move into array + + 1 move into observable
            }
        }
    }
    SECTION("observable with copied item + use_shared")
    {
        copy_count_tracker v{};
        auto               obs = rpp::source::just<rpp::memory_model::use_shared>(v);
        SECTION("subscribe on this observable")
        {
            obs.subscribe(mock.get_observer());
            SECTION("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 1); // 1 copy into shared_ptr
                CHECK(v.get_move_count() == 0);
            }
        }
    }
    SECTION("observable with moved item + use_shared")
    {
        copy_count_tracker v{};
        auto               obs = rpp::source::just<rpp::memory_model::use_shared>(std::move(v));
        SECTION("subscribe on this observable")
        {
            obs.subscribe(mock.get_observer());
            SECTION("value obtained")
            {
                CHECK(mock.get_on_next_const_ref_count() == 1);
                CHECK(mock.get_on_next_move_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
                CHECK(v.get_copy_count() == 0); // NOLINT
                CHECK(v.get_move_count() == 1); // 1 move into shared_ptr
            }
        }
    }
}

TEMPLATE_TEST_CASE("just variadic",
                   "",
                   std::pair<rpp::schedulers::current_thread, rpp::memory_model::use_stack>,
                   std::pair<rpp::schedulers::immediate, rpp::memory_model::use_stack>,
                   std::pair<rpp::schedulers::current_thread, rpp::memory_model::use_shared>,
                   std::pair<rpp::schedulers::immediate, rpp::memory_model::use_shared>)
{
    using memory_model = std::tuple_element_t<1, TestType>;
    using scheduler = std::tuple_element_t<0, TestType>;

    auto mock = mock_observer_strategy<int>();
    SECTION("observable just variadic")
    {
        auto obs = rpp::source::just<memory_model>(scheduler{}, 1, 2, 3, 4, 5, 6);
        SECTION("subscribe on it")
        {
            obs.subscribe(mock.get_observer());
            SECTION("observer obtains values in the same order")
            {
                CHECK(mock.get_received_values() == std::vector{ 1, 2, 3, 4, 5, 6 });
                CHECK(mock.get_on_completed_count() == 1);
            }

            SECTION("subscribe twice on same observer")
            {
                obs.subscribe(mock.get_observer());

                SECTION("observer obtains values in the same order twice")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6 });
                    CHECK(mock.get_on_completed_count() == 2);
                }
            }
        }
    }
}