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
#include <rpp/sources/create.hpp>
#include <exception>
#include "mock_observer.hpp"

TEST_CASE("subscribe as operator")
{
    mock_observer_strategy<int> mock{};
    auto observable = rpp::source::create<int>([](const auto& obs)
    {
        obs.on_next(1);
        obs.on_completed();
    });

    SECTION("subscribe observer")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(mock.get_observer())), void>);
        observable | rpp::operators::subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer())), rpp::disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::disposable_wrapper{}, mock.get_observer())), rpp::disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::disposable_wrapper{}, mock.get_observer());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe dynamic observer")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(mock.get_observer().as_dynamic())), void>);
        observable | rpp::operators::subscribe(mock.get_observer().as_dynamic());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe dynamic observer with disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer().as_dynamic())), rpp::disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe dynamic observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::disposable_wrapper{}, mock.get_observer().as_dynamic())), rpp::disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::disposable_wrapper{}, mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe lambdas")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe([](const auto&){}, [](const std::exception_ptr&){}, [](){})), void>);
        observable | rpp::operators::subscribe([&mock](const auto& v){ mock.on_next(v);}, [](const std::exception_ptr&){}, [](){});
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, [](const auto&){}, [](const std::exception_ptr&){}, [](){})), rpp::disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, [&mock](const auto& v){ mock.on_next(v);}, [](const std::exception_ptr&){}, [](){});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::disposable_wrapper{}, [](const auto&){}, [](const std::exception_ptr&){}, [](){})), rpp::disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::disposable_wrapper{}, [&mock](const auto& v){ mock.on_next(v);}, [](const std::exception_ptr&){}, [](){});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }
}

TEST_CASE("subscribe as member")
{
    mock_observer_strategy<int> mock{};
    auto observable = rpp::source::create<int>([](const auto& obs)
    {
        obs.on_next(1);
        obs.on_completed();
    });

    SECTION("subscribe observer")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(mock.get_observer())), void>);
        observable.subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer())), rpp::disposable_wrapper>);
        auto d = observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::disposable_wrapper{}, mock.get_observer())), rpp::disposable_wrapper>);
        auto d = observable.subscribe(rpp::disposable_wrapper{}, mock.get_observer());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe dynamic observer")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(mock.get_observer().as_dynamic())), void>);
        observable.subscribe(mock.get_observer().as_dynamic());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe dynamic observer with disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer().as_dynamic())), rpp::disposable_wrapper>);
        auto d = observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe dynamic observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::disposable_wrapper{}, mock.get_observer().as_dynamic())), rpp::disposable_wrapper>);
        auto d = observable.subscribe(rpp::disposable_wrapper{}, mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe lambdas")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe([](const auto&){}, [](const std::exception_ptr&){}, [](){})), void>);
        observable.subscribe([&mock](const auto& v){ mock.on_next(v);}, [](const std::exception_ptr&){}, [](){});
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, [](const auto&){}, [](const std::exception_ptr&){}, [](){})), rpp::disposable_wrapper>);
        auto d = observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, [&mock](const auto& v){ mock.on_next(v);}, [](const std::exception_ptr&){}, [](){});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::disposable_wrapper{}, [](const auto&){}, [](const std::exception_ptr&){}, [](){})), rpp::disposable_wrapper>);
        auto d = observable.subscribe(rpp::disposable_wrapper{}, [&mock](const auto& v){ mock.on_next(v);}, [](const std::exception_ptr&){}, [](){});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }
}
