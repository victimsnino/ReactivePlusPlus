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

#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/observers/mock_observer.hpp>
#include <rpp/operators/as_blocking.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/subjects/behavior_subject.hpp>
#include <rpp/subjects/publish_subject.hpp>
#include <rpp/subjects/replay_subject.hpp>

#include "copy_count_tracker.hpp"
#include "snitch_logging.hpp"

#include <thread>

TEST_CASE("publish subject multicasts values")
{
    auto mock_1 = mock_observer_strategy<int>{};
    auto mock_2 = mock_observer_strategy<int>{};
    SECTION("publish subject")
    {
        auto sub = rpp::subjects::publish_subject<int>{};
        SECTION("subscribe multiple observers")
        {
            auto dis_1 = rpp::composite_disposable_wrapper::make();
            auto dis_2 = rpp::composite_disposable_wrapper::make();
            sub.get_observable().subscribe(mock_1.get_observer(dis_1));
            sub.get_observable().subscribe(mock_2.get_observer(dis_2));

            SECTION("emit value")
            {
                sub.get_observer().on_next(1);
                SECTION("observers obtain value")
                {
                    auto validate = [](auto mock) {
                        CHECK(mock.get_received_values() == std::vector{1});
                        CHECK(mock.get_total_on_next_count() == 1);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    };
                    validate(mock_1);
                    validate(mock_2);
                }
            }
            SECTION("emit error")
            {
                sub.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
                SECTION("observers obtain error")
                {
                    auto validate = [](auto mock) {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 1);
                        CHECK(mock.get_on_completed_count() == 0);
                    };
                    validate(mock_1);
                    validate(mock_2);
                    SECTION("and then emission of on_next does nothing")
                    {
                        sub.get_observer().on_next(1);
                        validate(mock_1);
                        validate(mock_2);
                    }
                }
            }
            SECTION("emit on_completed")
            {
                sub.get_observer().on_completed();
                SECTION("observers obtain on_completed")
                {
                    auto validate = [](auto mock) {
                        CHECK(mock.get_total_on_next_count() == 0);
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 1);
                    };
                    validate(mock_1);
                    validate(mock_2);

                    SECTION("and then emission of on_next does nothing")
                    {
                        sub.get_observer().on_next(1);
                        validate(mock_1);
                        validate(mock_2);
                    }
                }
            }
            SECTION("emit multiple values")
            {
                SECTION("each sbuscriber obtain first value, then seconds and etc")
                {
                    sub.get_observer().on_next(1);
                    auto check_1 = [](auto mock) {
                        CHECK(mock.get_received_values() == std::vector{1});
                    };
                    check_1(mock_1);
                    check_1(mock_2);

                    sub.get_observer().on_next(2);
                    auto check_2 = [](auto mock) {
                        CHECK(mock.get_received_values() == std::vector{1, 2});
                    };
                    check_2(mock_1);
                    check_2(mock_2);
                }
            }
            SECTION("first subscriber unsubscribes and then emit value")
            {
                // 1 native, 1 inside subject
                // CHECK(dis_1.use_count() == 2);
                dis_1.dispose();
                // only this 1 native
                // CHECK(dis_1.use_count() == 1);

                sub.get_observer().on_next(1);
                SECTION("observers obtain value")
                {
                    CHECK(mock_1.get_total_on_next_count() == 0);
                    CHECK(mock_1.get_on_error_count() == 0);
                    CHECK(mock_1.get_on_completed_count() == 0);

                    CHECK(mock_2.get_received_values() == std::vector{1});
                    CHECK(mock_2.get_total_on_next_count() == 1);
                    CHECK(mock_2.get_on_error_count() == 0);
                    CHECK(mock_2.get_on_completed_count() == 0);
                }
            }
        }
    }
}

TEST_CASE("publish subject caches error/completed")
{
    auto mock = mock_observer_strategy<int>{};
    SECTION("publish subject")
    {
        auto subj = rpp::subjects::publish_subject<int>{};
        SECTION("emit value")
        {
            subj.get_observer().on_next(1);
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                SECTION("observer doesn't obtain value")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("emit error")
        {
            subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 1);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("emit on_completed")
        {
            subj.get_observer().on_completed();
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                SECTION("observer obtains on_completed")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
        SECTION("emit error and on_completed")
        {
            subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
            subj.get_observer().on_completed();
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                SECTION("observer obtains error")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 1);
                    CHECK(mock.get_on_completed_count() == 0);
                }
            }
        }
        SECTION("emit on_completed and error")
        {
            subj.get_observer().on_completed();
            subj.get_observer().on_error(std::make_exception_ptr(std::runtime_error{""}));
            SECTION("subscribe observer after emission")
            {
                subj.get_observable().subscribe(mock);
                SECTION("observer obtains on_completed")
                {
                    CHECK(mock.get_total_on_next_count() == 0);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 1);
                }
            }
        }
        SECTION("emit everything after on_completed via get_observer to avoid subscription")
        {
            auto observer = subj.get_observer();
            observer.on_completed();
            subj.get_observable().subscribe(mock);
            observer.on_next(1);
            observer.on_error(std::make_exception_ptr(std::runtime_error{""}));
            observer.on_completed();
            SECTION("no any calls except of cached on_completed")
            {
                CHECK(mock.get_total_on_next_count() == 0);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

TEMPLATE_TEST_CASE("serialized subjects handles race condition", "", rpp::subjects::serialized_publish_subject<int>, rpp::subjects::serialized_replay_subject<int>, rpp::subjects::serialized_behavior_subject<int>)
{
    auto subj = []() {
        if constexpr (std::same_as<TestType, rpp::subjects::serialized_behavior_subject<int>>)
            return TestType{0};
        else
            return TestType{};
    }();

    SECTION("call on_next from 2 threads")
    {
        bool on_error_called{};
        rpp::source::create<int>([&](auto&& obs) {
            subj.get_observable().subscribe(std::forward<decltype(obs)>(obs));
            subj.get_observer().on_next(1);
        })
            | rpp::operators::as_blocking()
            | rpp::operators::subscribe([&](int) {
            CHECK(!on_error_called);
            std::thread{[&]{ subj.get_observer().on_error({}); }}.detach();

            std::this_thread::sleep_for(std::chrono::seconds{1});
            CHECK(!on_error_called); },
                                        [&](const std::exception_ptr&) { on_error_called = true; });

        CHECK(on_error_called);
    }
}

TEMPLATE_TEST_CASE("replay subject multicasts values and replay", "", rpp::subjects::replay_subject<int>, rpp::subjects::serialized_replay_subject<int>)
{
    SECTION("replay subject")
    {
        auto mock_1 = mock_observer_strategy<int>{};
        auto mock_2 = mock_observer_strategy<int>{};
        auto mock_3 = mock_observer_strategy<int>{};

        auto sub = TestType{};

        SECTION("subscribe multiple observers")
        {
            sub.get_observable().subscribe(mock_1.get_observer());
            sub.get_observable().subscribe(mock_2.get_observer());

            sub.get_observer().on_next(1);
            sub.get_observer().on_next(2);
            sub.get_observer().on_next(3);

            SECTION("observers obtain values")
            {
                auto validate = [](auto mock) {
                    CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                    CHECK(mock.get_total_on_next_count() == 3);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                };
                validate(mock_1);
                validate(mock_2);
            }

            sub.get_observable().subscribe(mock_3.get_observer());

            SECTION("observer obtains replayed values")
            {
                CHECK(mock_3.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock_3.get_total_on_next_count() == 3);
                CHECK(mock_3.get_on_error_count() == 0);
                CHECK(mock_3.get_on_completed_count() == 0);
            }

            sub.get_observer().on_next(4);

            SECTION("observers stil obtain values")
            {
                auto validate = [](auto mock) {
                    CHECK(mock.get_received_values() == std::vector{1, 2, 3, 4});
                    CHECK(mock.get_total_on_next_count() == 4);
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                };
                validate(mock_1);
                validate(mock_2);
                validate(mock_3);
            }
        }
    }

    SECTION("bounded replay subject")
    {
        auto mock_1 = mock_observer_strategy<int>{};
        auto mock_2 = mock_observer_strategy<int>{};

        size_t bound = 1;
        auto   sub   = TestType{bound};

        SECTION("subscribe multiple observers")
        {
            sub.get_observable().subscribe(mock_1.get_observer());

            sub.get_observer().on_next(1);
            sub.get_observer().on_next(2);
            sub.get_observer().on_next(3);

            SECTION("observer obtains values")
            {
                CHECK(mock_1.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock_1.get_total_on_next_count() == 3);
                CHECK(mock_1.get_on_error_count() == 0);
                CHECK(mock_1.get_on_completed_count() == 0);
            }

            sub.get_observable().subscribe(mock_2.get_observer());

            SECTION("observer obtains latest replayed values")
            {
                CHECK(mock_2.get_received_values() == std::vector{3});
                CHECK(mock_2.get_total_on_next_count() == 1);
                CHECK(mock_2.get_on_error_count() == 0);
                CHECK(mock_2.get_on_completed_count() == 0);
            }
        }
    }

    SECTION("bounded replay subject with duration")
    {
        using namespace std::chrono_literals;

        auto mock_1 = mock_observer_strategy<int>{};
        auto mock_2 = mock_observer_strategy<int>{};

        size_t bound    = 2;
        auto   duration = 5ms;
        auto   sub      = TestType{bound, duration};

        SECTION("subscribe multiple observers")
        {
            sub.get_observable().subscribe(mock_1.get_observer());

            sub.get_observer().on_next(1);
            sub.get_observer().on_next(2);
            sub.get_observer().on_next(3);

            SECTION("observer obtains values")
            {
                CHECK(mock_1.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock_1.get_total_on_next_count() == 3);
                CHECK(mock_1.get_on_error_count() == 0);
                CHECK(mock_1.get_on_completed_count() == 0);
            }

            std::this_thread::sleep_for(duration);

            sub.get_observable().subscribe(mock_2.get_observer());

            SECTION("subject replay only non expired values")
            {
                CHECK(mock_2.get_received_values() == std::vector<int>{});
                CHECK(mock_2.get_total_on_next_count() == 0);
                CHECK(mock_2.get_on_error_count() == 0);
                CHECK(mock_2.get_on_completed_count() == 0);
            }
        }
    }
}

TEMPLATE_TEST_CASE("replay subject doesn't introduce additional copies", "", rpp::subjects::replay_subject<copy_count_tracker>, rpp::subjects::serialized_replay_subject<copy_count_tracker>)
{
    SECTION("on_next by rvalue")
    {
        auto sub = TestType{};

        sub.get_observable().subscribe([](copy_count_tracker tracker) { // NOLINT
            CHECK(tracker.get_copy_count() == 2);                       // 1 copy to internal replay buffer + 1 copy to this observer
            CHECK(tracker.get_move_count() == 0);
        });

        sub.get_observer().on_next(copy_count_tracker{});

        sub.get_observable().subscribe([](copy_count_tracker tracker) { // NOLINT
            CHECK(tracker.get_copy_count() == 2 + 1);                   // + 1 copy values from buffer for this observer
            CHECK(tracker.get_move_count() == 0 + 1);                   // + 1 move to this observer
        });
    }

    SECTION("on_next by lvalue")
    {
        copy_count_tracker tracker{};
        auto               sub = TestType{};

        sub.get_observable().subscribe([](copy_count_tracker tracker) { // NOLINT
            CHECK(tracker.get_copy_count() == 2);                       // 1 copy to internal replay buffer + 1 copy to this observer
            CHECK(tracker.get_move_count() == 0);
        });

        sub.get_observer().on_next(tracker);

        sub.get_observable().subscribe([](copy_count_tracker tracker) { // NOLINT
            CHECK(tracker.get_copy_count() == 2 + 1);                   // + 1 copy values from buffer for this observer
            CHECK(tracker.get_move_count() == 0 + 1);                   // + 1 move to this observer
        });
    }
}

TEMPLATE_TEST_CASE("replay subject multicasts values and replay", "", rpp::subjects::behavior_subject<int>, rpp::subjects::serialized_behavior_subject<int>)
{
    const auto mock_1 = mock_observer_strategy<int>{};
    const auto subj   = TestType{10};

    CHECK(subj.get_value() == 10);

    SECTION("subscribe to subject with default")
    {
        subj.get_observable().subscribe(mock_1);
        CHECK(mock_1.get_received_values() == std::vector<int>{10});

        SECTION("emit value and subscribe other observer")
        {
            const auto mock_2 = mock_observer_strategy<int>{};

            subj.get_observer().on_next(5);
            CHECK(subj.get_value() == 5);

            CHECK(mock_1.get_received_values() == std::vector<int>{10, 5});
            CHECK(mock_2.get_received_values() == std::vector<int>{});

            subj.get_observable().subscribe(mock_2);

            CHECK(mock_2.get_received_values() == std::vector<int>{5});

            SECTION("emit one more value and subscribe one more other observer")
            {
                const auto mock_3 = mock_observer_strategy<int>{};
                subj.get_observer().on_next(1);
                CHECK(subj.get_value() == 1);

                CHECK(mock_1.get_received_values() == std::vector<int>{10, 5, 1});
                CHECK(mock_2.get_received_values() == std::vector<int>{5, 1});
                CHECK(mock_3.get_received_values() == std::vector<int>{});

                subj.get_observable().subscribe(mock_3);

                CHECK(mock_3.get_received_values() == std::vector<int>{1});
            }
        }

        SECTION("subject keeps error")
        {
            subj.get_observer().on_error(std::exception_ptr{});
            CHECK(mock_1.get_on_error_count() == 1);

            const auto mock_4 = mock_observer_strategy<int>{};
            subj.get_observable().subscribe(mock_4);

            CHECK(mock_4.get_received_values() == std::vector<int>{});
            CHECK(mock_4.get_on_error_count() == 1);
            CHECK(mock_4.get_on_completed_count() == 0);
        }
    }
}
