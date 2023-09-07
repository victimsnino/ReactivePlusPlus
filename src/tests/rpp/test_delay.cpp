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

#include <rpp/sources/just.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <rpp/operators/delay.hpp>
#include <rpp/operators/as_blocking.hpp>

#include "mock_observer.hpp"
#include "disposable_observable.hpp"

namespace
{
    rpp::schedulers::details::schedulables_queue test_queue{};

    class manual_scheduler final
    {
    public:
        class worker_strategy
        {
        public:
            template<rpp::schedulers::constraint::schedulable_handler Handler, typename...Args, rpp::schedulers::constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_for(rpp::schedulers::duration duration, Fn&& fn, Handler&& handler, Args&&...args) const
            {
                test_queue.emplace(rpp::schedulers::time_point{duration}, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

            static rpp::disposable_wrapper get_disposable() {return rpp::disposable_wrapper{std::make_shared<rpp::composite_disposable>()}; }
            static rpp::schedulers::time_point now() { return rpp::schedulers::clock_type::now(); }
        };

        static rpp::schedulers::worker<worker_strategy> create_worker()
        {
            test_queue = rpp::schedulers::details::schedulables_queue{};
            return rpp::schedulers::worker<worker_strategy>{};
        }
    };
}
TEST_CASE("delay delays observable's emissions")
{
    auto mock = mock_observer_strategy<int>{};
    std::chrono::milliseconds delay_duration{300};

    SECTION("observable of -1-|")
    {
        const auto now  = rpp::schedulers::clock_type::now();

        rpp::source::just(1)
                | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
                | rpp::ops::as_blocking()
                | rpp::ops::subscribe(
                           [&](const auto& v)
                           {
                               SECTION("should see event after the delay")
                               {
                                   CHECK(rpp::schedulers::clock_type::now() >= now + delay_duration);
                               }

                               mock.on_next(v);
                           },
                           [&](const std::exception_ptr& err) { mock.on_error(err); },
                           [&]()
                           {
                               SECTION("should see event after the delay")
                               {
                                   CHECK(rpp::schedulers::clock_type::now() >= now + delay_duration);
                               }

                               mock.on_completed();
                           });

        SECTION("should see -1-|")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    SECTION("observable of -x")
    {
        const auto now  = rpp::schedulers::clock_type::now();

        rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
                | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
                | rpp::ops::as_blocking()
                | rpp::ops::subscribe([&](auto&&                    v) { mock.on_next(v); },
                           [&](const std::exception_ptr& err)
                           {
                               SECTION("should see event immediately")
                               {
                                   CHECK(rpp::schedulers::clock_type::now() < now + delay_duration);
                               }
                               mock.on_error(err);
                           },
                           [&]() { mock.on_completed(); });

        SECTION("should see -x after the delay")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }

    SECTION("observable of -|")
    {
        const auto now  = rpp::schedulers::clock_type::now();

        rpp::source::empty<int>()
                | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
                | rpp::ops::as_blocking()
                | rpp::ops::subscribe([&](auto&&                    v) { mock.on_next(v); },
                           [&](const std::exception_ptr& err) { mock.on_error(err); },
                           [&]()
                           {
                               SECTION("should see event after delay")
                               {
                                   CHECK(rpp::schedulers::clock_type::now() >= now + delay_duration);
                               }
                               mock.on_completed();
                           });

        SECTION("should see -|")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    SECTION("subject with items")
    {
        auto subj = rpp::subjects::publish_subject<int>{};

        SECTION("subscribe on subject via delay and doing recursive submit from another thread")
        {
            SECTION("all values obtained in the same thread")
            {
                auto current_thread = std::this_thread::get_id();

                subj.get_observable()
                    | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
                    | rpp::ops::subscribe([&](int v)
                    {
                        CHECK(std::this_thread::get_id() == current_thread);

                        mock.on_next(v);

                        if (v == 1)
                        {
                            std::thread{[&]{subj.get_observer().on_next(2);}}.join();

                            SECTION("no recursive on_next calls")
                            {
                                CHECK(mock.get_received_values() == std::vector{1});
                            }
                        }
                    });

                subj.get_observer().on_next(1);

                SECTION("all values obtained")
                {
                    CHECK(mock.get_received_values() == std::vector{ 1, 2 });
                }
            }
        }
        SECTION("subscribe on subject via delay via test_scheduler, sent value")
        {
            subj.get_observable()
                | rpp::ops::delay(std::chrono::seconds{30000}, manual_scheduler{})
                | rpp::ops::subscribe(mock.get_observer());

            subj.get_observer().on_next(1);

            SECTION("no memory leak")
            {
                test_queue = rpp::schedulers::details::schedulables_queue{};
                // checked via sanitizer
            }
        }
    }

    SECTION("observable of -1-| but with invoking schedulable after subscription")
    {
        rpp::source::just(1)
            | rpp::ops::delay(std::chrono::seconds{0}, manual_scheduler{})
            | rpp::ops::subscribe(mock.get_observer());

        SECTION("shouldn't see anything before manual invoking")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 0);
        }

        REQUIRE(!test_queue.is_empty());
        auto top = test_queue.top();
        test_queue.pop();
        (*top)();

        SECTION("should see -1-|")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
            CHECK(test_queue.is_empty());
        }
    }
    SECTION("observable of -1-x but with invoking schedulable after subscription")
    {
        rpp::source::create<int>([](const auto& obs)
        {
            obs.on_next(1);
            obs.on_error({});
        })
        | rpp::ops::delay(std::chrono::seconds{0}, manual_scheduler{})
        | rpp::ops::subscribe(mock.get_observer());

        SECTION("shouldn't see anything before manual invoking")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 0);
        }

        REQUIRE(!test_queue.is_empty());
        auto top = test_queue.top();
        test_queue.pop();
        (*top)();

        SECTION("should see -x")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
            CHECK(test_queue.is_empty());
        }
    }
}

TEST_CASE("delay disposes original disposable on disposing")
{
    test_operator_with_disposable<int>(rpp::ops::delay(std::chrono::seconds{0}, manual_scheduler{}));
}