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

#include <rpp/observables/dynamic_observable.hpp>
#include <rpp/operators/switch_on_next.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/error.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/never.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include "copy_count_tracker.hpp"
#include "disposable_observable.hpp"
#include "mock_observer.hpp"
#include "snitch_logging.hpp"

TEST_CASE("switch_on_next switches observable after obtaining new one")
{
    auto mock = mock_observer_strategy<int>();
    SECTION("just observable of just observables")
    {
        auto observable = rpp::source::just(rpp::source::just(1), rpp::source::just(2), rpp::source::just(3));
        SECTION("subscribe on it via switch_on_next")
        {
            observable | rpp::ops::switch_on_next() | rpp::ops::subscribe(mock);
            SECTION("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    SECTION("just observable of just observables where second is error")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                            rpp::source::error<int>(std::make_exception_ptr(std::runtime_error{""})).as_dynamic(),
                                            rpp::source::just(3).as_dynamic());
        SECTION("subscribe on it via switch_on_next")
        {
            observable | rpp::ops::switch_on_next() | rpp::ops::subscribe(mock);
            SECTION("obtains values as from concat but stops on error")
            {
                CHECK(mock.get_received_values() == std::vector{1});
                CHECK(mock.get_on_error_count() == 1);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
    SECTION("just observable of just observables where second is completed")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                            rpp::source::empty<int>().as_dynamic(),
                                            rpp::source::just(3).as_dynamic());
        SECTION("subscribe on it via switch_on_next")
        {
            observable | rpp::ops::switch_on_next() | rpp::ops::subscribe(mock);
            SECTION("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    SECTION("just observable of just observables where second is never")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                            rpp::source::never<int>().as_dynamic(),
                                            rpp::source::just(3).as_dynamic());
        SECTION("subscribe on it via switch_on_next")
        {
            observable | rpp::ops::switch_on_next() | rpp::ops::subscribe(mock);
            SECTION("obtains values as from concat")
            {
                CHECK(mock.get_received_values() == std::vector{1, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
    SECTION("just observable of just observables where last is never")
    {
        auto observable = rpp::source::just(rpp::source::just(1).as_dynamic(),
                                            rpp::source::just(3).as_dynamic(),
                                            rpp::source::never<int>().as_dynamic());
        SECTION("subscribe on it via switch_on_next")
        {
            observable | rpp::ops::switch_on_next() | rpp::ops::subscribe(mock);
            SECTION("obtains values as from concat but no complete")
            {
                CHECK(mock.get_received_values() == std::vector{1, 3});
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 0);
            }
        }
    }
    SECTION("subject of just subjects")
    {
        auto subj_1           = rpp::subjects::publish_subject<int>();
        auto subj_2           = rpp::subjects::publish_subject<int>();
        auto subj_of_subjects = rpp::subjects::publish_subject<rpp::dynamic_observable<int>>();

        SECTION("subscribe on it via switch_on_next")
        {
            subj_of_subjects.get_observable() | rpp::ops::switch_on_next() | rpp::ops::subscribe(mock);
            SECTION("send first subject and send values for all subjects")
            {
                subj_1.get_observer().on_next(0);

                subj_of_subjects.get_observer().on_next(subj_1.get_observable().as_dynamic());

                SECTION("Only value from first subject obtained")
                {
                    subj_1.get_observer().on_next(1);
                    subj_2.get_observer().on_next(2);

                    CHECK(mock.get_received_values() == std::vector{1});
                    CHECK(mock.get_on_error_count() == 0);
                    CHECK(mock.get_on_completed_count() == 0);
                }
                SECTION("send second subject and send values for all subjects")
                {
                    subj_of_subjects.get_observer().on_next(subj_2.get_observable().as_dynamic());

                    SECTION("Only value from second subject obtained")
                    {
                        subj_1.get_observer().on_next(1);
                        subj_2.get_observer().on_next(2);

                        CHECK(mock.get_received_values() == std::vector{2});
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    }
                }
                SECTION("original subject completes but provided send value")
                {
                    subj_of_subjects.get_observer().on_completed();
                    subj_1.get_observer().on_next(1);
                    subj_2.get_observer().on_next(2);
                    SECTION("value obtained")
                    {
                        CHECK(mock.get_received_values() == std::vector{1});
                        CHECK(mock.get_on_error_count() == 0);
                        CHECK(mock.get_on_completed_count() == 0);
                    }
                    SECTION("subject sends on_completed")
                    {
                        subj_1.get_observer().on_completed();
                        SECTION("subsriber completed")
                        {
                            CHECK(mock.get_on_completed_count() == 1);
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE("switch_on_next doesn't produce extra copies")
{
    SECTION("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto               obs = rpp::source::just(verifier.get_observable()) | rpp::ops::switch_on_next();
        SECTION("subscribe")
        {
            obs | rpp::ops::subscribe([](copy_count_tracker) {}); // NOLINT
            SECTION("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 1); // 1 copy to final lambda
                REQUIRE(verifier.get_move_count() == 0);
            }
        }
    }
}

TEST_CASE("switch_on_next doesn't produce extra copies for move")
{
    SECTION("observable and subscriber")
    {
        copy_count_tracker verifier{};
        auto               obs = rpp::source::just(verifier.get_observable_for_move()) | rpp::ops::switch_on_next();
        SECTION("subscribe")
        {
            obs | rpp::ops::subscribe([](copy_count_tracker) {}); // NOLINT
            SECTION("no extra copies")
            {
                REQUIRE(verifier.get_copy_count() == 0);
                REQUIRE(verifier.get_move_count() == 1); // 1 move to final lambda
            }
        }
    }
}

TEST_CASE("switch_on_next handles race condition")
{
    SECTION("source observable in current thread pairs with error in other thread")
    {
        std::atomic_bool on_error_called{false};
        auto             subject = rpp::subjects::publish_subject<rpp::dynamic_observable<int>>{};
        SECTION("subscribe on it")
        {
            SECTION("on_error can't interleave with on_next")
            {
                std::thread th{};

                subject.get_observable()
                    | rpp::ops::switch_on_next()
                    | rpp::ops::subscribe([&](auto&&) {
                                       CHECK(!on_error_called);
                                       th = std::thread{[&]
                                       {
                                           subject.get_observer().on_error(std::exception_ptr{});
                                       }};
                                       std::this_thread::sleep_for(std::chrono::seconds{1});
                                       CHECK(!on_error_called); },
                                          [&](auto) { on_error_called = true; });

                subject.get_observer().on_next(rpp::source::just(1));

                th.join();
                CHECK(on_error_called);
            }
        }
    }
}