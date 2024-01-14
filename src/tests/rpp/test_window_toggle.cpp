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

#include <rpp/operators/window_toggle.hpp>
#include <rpp/sources/just.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/never.hpp>

#include "mock_observer.hpp"
#include "disposable_observable.hpp"
#include "rpp/schedulers/immediate.hpp"
#include "snitch_logging.hpp"


TEST_CASE("window_toggle")
{
    mock_observer_strategy<rpp::window_toggle_observable<int>> mock{};
    std::vector<mock_observer_strategy<int>> inner_mocks{}; 

    auto subscribe_mocks = [&mock, &inner_mocks](auto&& observable)
    {
        observable.subscribe([&mock, &inner_mocks](const rpp::window_toggle_observable<int>& observable)
        {
            mock.on_next(observable);
            observable.subscribe(inner_mocks.emplace_back());
        },
        [&mock](const std::exception_ptr& err) { mock.on_error(err); },
        [&mock]() { mock.on_completed(); });
    };

    SECTION("opening - just(1), closing - never()")
    {
        subscribe_mocks(rpp::source::just(1,2,3) 
            | rpp::ops::window_toggle(rpp::source::just(1), [](int){return rpp::source::never<int>();}));
        
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
        REQUIRE(inner_mocks.size() == 1);
        for (const auto& inner : inner_mocks)
        {
            CHECK(inner.get_received_values() == std::vector<int>{1,2,3});
            CHECK(inner.get_on_error_count() == 0);
            CHECK(inner.get_on_completed_count() == 1);
        }
    }
    SECTION("opening - just(1), closing - empty()")
    {
        subscribe_mocks(rpp::source::just(1,2,3) 
            | rpp::ops::window_toggle(rpp::source::just(1), [](int){return rpp::source::empty<int>();}));
        
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
        REQUIRE(inner_mocks.size() == 1);
        for (const auto& inner : inner_mocks)
        {
            CHECK(inner.get_received_values() == std::vector<int>{});
            CHECK(inner.get_on_error_count() == 0);
            CHECK(inner.get_on_completed_count() == 1);
        }
    }
    SECTION("opening - just(1,2,3), closing - empty()")
    {
        subscribe_mocks(rpp::source::just(1,2,3) 
            | rpp::ops::window_toggle(rpp::source::just(1,2,3), [](int){return rpp::source::empty<int>();}));
        
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
        REQUIRE(inner_mocks.size() == 3);
        for (const auto& inner : inner_mocks)
        {
            CHECK(inner.get_received_values() == std::vector<int>{});
            CHECK(inner.get_on_error_count() == 0);
            CHECK(inner.get_on_completed_count() == 1);
        }
    }
    SECTION("opening - just(1,2,3), closing - never()")
    {
        subscribe_mocks(rpp::source::just(1,2,3) 
            | rpp::ops::window_toggle(rpp::source::just(1,2,3), [](int){return rpp::source::never<int>();}));
        
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
        REQUIRE(inner_mocks.size() == 3);
        for (const auto& inner : inner_mocks)
        {
            CHECK(inner.get_on_error_count() == 0);
            CHECK(inner.get_on_completed_count() == 1);
        }
        CHECK(inner_mocks[0].get_received_values() == std::vector<int>{1,2,3});
        CHECK(inner_mocks[1].get_received_values() == std::vector<int>{2,3});
        CHECK(inner_mocks[2].get_received_values() == std::vector<int>{3});
    }
    SECTION("opening - just(1,2,3), closing - just(1)")
    {
        subscribe_mocks(rpp::source::just(1,2,3) 
            | rpp::ops::window_toggle(rpp::source::just(1,2,3), [](int){return rpp::source::just(1);}));
        
        CHECK(mock.get_on_error_count() == 0);
        CHECK(mock.get_on_completed_count() == 1);
        REQUIRE(inner_mocks.size() == 3);
        for (const auto& inner : inner_mocks)
        {
            CHECK(inner.get_on_error_count() == 0);
            CHECK(inner.get_on_completed_count() == 1);
        }
        CHECK(inner_mocks[0].get_received_values() == std::vector<int>{1});
        CHECK(inner_mocks[1].get_received_values() == std::vector<int>{2});
        CHECK(inner_mocks[2].get_received_values() == std::vector<int>{3});
    }
}

TEST_CASE("window_toggle disposes original disposable only when everything is disposed")
{
    auto source_disposable = std::make_shared<rpp::composite_disposable>();
    auto obs = rpp::source::create<int>([source_disposable](auto&& obs)
    {
        obs.set_upstream(source_disposable);
        obs.on_next(1);
    });

    auto observer_disposable = std::make_shared<rpp::composite_disposable>();
    auto inner_observer_disposable = std::make_shared<rpp::composite_disposable>();
    obs 
        | rpp::ops::window_toggle(rpp::source::just(rpp::schedulers::immediate{}, 1), [](int){return rpp::source::never<int>(); }) 
        | rpp::ops::subscribe(rpp::composite_disposable_wrapper{observer_disposable}, [inner_observer_disposable](const rpp::window_toggle_observable<int>& new_obs)
    {
        new_obs.subscribe(rpp::composite_disposable_wrapper{inner_observer_disposable}, [](int){});
    });

    CHECK(!source_disposable->is_disposed());
    CHECK(!observer_disposable->is_disposed());
    CHECK(!inner_observer_disposable->is_disposed());

    observer_disposable->dispose();

    CHECK(!source_disposable->is_disposed());
    CHECK(observer_disposable->is_disposed());
    CHECK(!inner_observer_disposable->is_disposed());

    inner_observer_disposable->dispose();

    CHECK(source_disposable->is_disposed());
    CHECK(observer_disposable->is_disposed());
    CHECK(inner_observer_disposable->is_disposed());
}

TEST_CASE("window_toggle satisfies disposable contracts")
{
    test_operator_with_disposable<int>(rpp::ops::window_toggle(rpp::source::just(1), [](int){return rpp::source::just(1); }));
}