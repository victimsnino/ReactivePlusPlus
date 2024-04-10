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

#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/observers/mock_observer.hpp>
#include <rpp/sources/just.hpp>

#include <exception>

TEMPLATE_TEST_CASE("subscribe as operator", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    mock_observer_strategy<int> mock{};
    auto                        observable = rpp::source::just<TestType>(1);

    SECTION("subscribe observer strategy")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(mock)), void>);
        observable | rpp::operators::subscribe(mock);
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer strategy with disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), mock)), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), mock);
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer strategy with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), mock)), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), mock);
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe observer")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(mock.get_observer())), void>);
        observable | rpp::operators::subscribe(mock.get_observer());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer())), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer())), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer());
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
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer().as_dynamic())), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe dynamic observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer().as_dynamic())), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe lambdas")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{})), void>);
        observable | rpp::operators::subscribe([&mock](const auto& v) { mock.on_next(v); }, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{});
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{})), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::make(), [&mock](const auto& v) { mock.on_next(v); }, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{})), rpp::composite_disposable_wrapper>);
        auto d = observable | rpp::operators::subscribe(rpp::composite_disposable_wrapper::empty(), [&mock](const auto& v) { mock.on_next(v); }, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }
}

TEMPLATE_TEST_CASE("subscribe as member", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    mock_observer_strategy<int> mock{};
    auto                        observable = rpp::source::just<TestType>(1);

    SECTION("subscribe observer strategy")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(mock)), void>);
        observable.subscribe(mock);
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer strategy with disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::make(), mock)), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(rpp::composite_disposable_wrapper::make(), mock);
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer strategy with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::empty(), mock)), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(rpp::composite_disposable_wrapper::empty(), mock);
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe observer")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(mock.get_observer())), void>);
        observable.subscribe(mock);
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer())), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer())), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer());
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
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer().as_dynamic())), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(rpp::composite_disposable_wrapper::make(), mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe dynamic observer with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer().as_dynamic())), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(rpp::composite_disposable_wrapper::empty(), mock.get_observer().as_dynamic());
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }

    SECTION("subscribe lambdas")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{})), void>);
        observable.subscribe([&mock](const auto& v) { mock.on_next(v); }, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{});
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::make(), rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{})), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(
            rpp::composite_disposable_wrapper::make(),
            [&mock](const auto& v) { mock.on_next(v); },
            rpp::utils::empty_function_t<std::exception_ptr>{},
            rpp::utils::empty_function_t<>{});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values() == std::vector{1});
    }

    SECTION("subscribe lambdas with disposed disposable")
    {
        static_assert(std::is_same_v<decltype(observable.subscribe(rpp::composite_disposable_wrapper::empty(), rpp::utils::empty_function_t<int>{}, rpp::utils::empty_function_t<std::exception_ptr>{}, rpp::utils::empty_function_t<>{})), rpp::composite_disposable_wrapper>);
        auto d = observable.subscribe(
            rpp::composite_disposable_wrapper::empty(),
            [&mock](const auto& v) { mock.on_next(v); },
            rpp::utils::empty_function_t<std::exception_ptr>{},
            rpp::utils::empty_function_t<>{});
        CHECK(d.is_disposed());
        CHECK(mock.get_received_values().empty());
    }
}
