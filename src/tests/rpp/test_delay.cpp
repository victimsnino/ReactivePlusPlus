//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/as_blocking.hpp>
#include <rpp/operators/delay.hpp>
#include <rpp/operators/observe_on.hpp>
#include <rpp/schedulers/test_scheduler.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "disposable_observable.hpp"

namespace
{

    class manual_scheduler final
    {
    public:
        class worker_strategy
        {
        public:
            inline static rpp::schedulers::details::schedulables_queue<worker_strategy> s_test_queue{};
            template<rpp::schedulers::constraint::schedulable_handler Handler, typename... Args, rpp::schedulers::constraint::schedulable_fn<Handler, Args...> Fn>
            void defer_for(rpp::schedulers::duration duration, Fn&& fn, Handler&& handler, Args&&... args) const
            {
                s_test_queue.emplace(rpp::schedulers::time_point{duration}, std::forward<Fn>(fn), std::forward<Handler>(handler), std::forward<Args>(args)...);
            }

            static rpp::disposable_wrapper     get_disposable() { return rpp::disposable_wrapper::make<rpp::composite_disposable>(); }
            static rpp::schedulers::time_point now() { return rpp::schedulers::clock_type::now(); }
        };

        static rpp::schedulers::worker<worker_strategy> create_worker()
        {
            worker_strategy::s_test_queue = rpp::schedulers::details::schedulables_queue<worker_strategy>{};
            return rpp::schedulers::worker<worker_strategy>{};
        }
    };
} // namespace
TEST_CASE("delay delays observable's emissions")
{
    auto                      mock = mock_observer_strategy<int>{};
    std::chrono::milliseconds delay_duration{300};
    auto                      scheduler = test_scheduler{};

    auto subscribe_with_delay = [&](auto get_now) {
        const auto now = get_now();
        return rpp::ops::subscribe(
            [&, now, get_now](const auto& v) {
                SECTION("should see event after the delay")
                CHECK(get_now() >= now + delay_duration);

                mock.on_next(v);
            },
            [&, now, get_now](const std::exception_ptr& err) {
                SECTION("should see event after the delay")
                CHECK(get_now() >= now + delay_duration);
                mock.on_error(err);
            },
            [&, now, get_now]() {
                SECTION("should see event after the delay")
                CHECK(get_now() >= now + delay_duration);

                mock.on_completed();
            });
    };

    SECTION("observable of -1-|")
    {
        rpp::source::just(1)
            | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
            | rpp::ops::as_blocking()
            | subscribe_with_delay([]() { return rpp::schedulers::clock_type::now(); });

        SECTION("should see -1-|")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }

    SECTION("observable of -x")
    {
        rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""}))
            | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
            | rpp::ops::as_blocking()
            | subscribe_with_delay([]() { return rpp::schedulers::clock_type::now(); });

        SECTION("should see -x after the delay")
        {
            CHECK(mock.get_received_values().empty());
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }

    SECTION("observable of -|")
    {
        rpp::source::empty<int>()
            | rpp::ops::delay(delay_duration, rpp::schedulers::current_thread{})
            | rpp::ops::as_blocking()
            | subscribe_with_delay([]() { return rpp::schedulers::clock_type::now(); });

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
                    | rpp::ops::subscribe([&](int v) {
                          CHECK(std::this_thread::get_id() == current_thread);

                          mock.on_next(v);

                          if (v == 1)
                          {
                              std::thread{[&] {
                                  subj.get_observer().on_next(2);
                              }}.join();

                              SECTION("no recursive on_next calls")
                              {
                                  CHECK(mock.get_received_values() == std::vector{1});
                              }
                          }
                      });

                subj.get_observer().on_next(1);

                SECTION("all values obtained")
                {
                    CHECK(mock.get_received_values() == std::vector{1, 2});
                }
            }
        }
        SECTION("subscribe on subject via delay via test_scheduler, sent value")
        {
            subj.get_observable()
                | rpp::ops::delay(std::chrono::seconds{30000}, manual_scheduler{})
                | subscribe_with_delay([]() { return rpp::schedulers::clock_type::now(); });

            subj.get_observer().on_next(1);

            SECTION("no memory leak")
            {
                manual_scheduler::worker_strategy::s_test_queue = rpp::schedulers::details::schedulables_queue<manual_scheduler::worker_strategy>{};
                // checked via sanitizer
            }
        }
    }

    SECTION("observable of -1-| but with invoking schedulable after subscription")
    {
        rpp::source::just(1)
            | rpp::ops::delay(delay_duration, scheduler)
            | subscribe_with_delay([]() { return test_scheduler::worker_strategy::now(); });

        SECTION("shouldn't see anything before manual invoking")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 0);
        }

        SECTION("should see nothing before reaching of time")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 0);
        }

        scheduler.time_advance(delay_duration);

        SECTION("should see -1-|")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1});
            CHECK(mock.get_on_completed_count() == 1);
            CHECK(mock.get_on_error_count() == 0);
        }
    }
    SECTION("observable of -1-x but with invoking schedulable after subscription")
    {
        rpp::source::create<int>([](const auto& obs) {
            obs.on_next(1);
            obs.on_error({});
        })
            | rpp::ops::delay(delay_duration, scheduler)
            | subscribe_with_delay([]() { return test_scheduler::worker_strategy::now(); });

        SECTION("shouldn't see anything before manual invoking")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 0);
        }

        scheduler.time_advance(delay_duration);

        SECTION("should see --1-x")
        {
            CHECK(mock.get_received_values() == std::vector<int>{1});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }
    }
}

TEST_CASE("delay satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::delay(std::chrono::seconds{0}, manual_scheduler{}));
}

TEST_CASE("observe_on forward error immediately")
{
    auto                      mock = mock_observer_strategy<int>{};
    std::chrono::milliseconds delay_duration{300};
    auto                      scheduler = test_scheduler{};

    auto subscribe_with_delay = [&](auto get_now) {
        const auto now = get_now();
        return rpp::ops::subscribe(
            [&, now, get_now](const auto& v) {
                SECTION("should see event after the delay")
                CHECK(get_now() >= now + delay_duration);

                mock.on_next(v);
            },
            [&, now, get_now](const std::exception_ptr& err) {
                SECTION("should see event after the delay")
                CHECK(get_now() == now);
                mock.on_error(err);
            },
            [&, now, get_now]() {
                SECTION("should see event after the delay")
                CHECK(get_now() >= now + delay_duration);

                mock.on_completed();
            });
    };

    SECTION("observable of -1-x but with invoking schedulable after subscription")
    {
        const auto now = test_scheduler::worker_strategy::now();
        rpp::source::create<int>([](const auto& obs) {
            obs.on_next(1);
            obs.on_error({});
        })
            | rpp::ops::observe_on(scheduler, delay_duration)
            | subscribe_with_delay([]() { return test_scheduler::worker_strategy::now(); });

        SECTION("should see on_error immediately")
        {
            CHECK(mock.get_received_values() == std::vector<int>{});
            CHECK(mock.get_on_completed_count() == 0);
            CHECK(mock.get_on_error_count() == 1);
        }

        scheduler.time_advance(delay_duration);

        CHECK(scheduler.get_schedulings() == std::vector{now + delay_duration});
        CHECK(scheduler.get_executions() == std::vector<rpp::schedulers::time_point>{});
    }
}
