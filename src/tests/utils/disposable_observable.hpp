//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <snitch/snitch.hpp>

#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/observers/dynamic_observer.hpp>
#include <rpp/sources/create.hpp>

template<typename T>
auto observable_with_disposable(rpp::disposable_wrapper d)
{
    return rpp::source::create<T>([d](auto&& obs) {
        obs.set_upstream(d);
    });
}

template<rpp::constraint::decayed_type Type, rpp::details::observables::constraint::disposable_strategy Strategy>
struct wrapped_observable_strategy_set_upstream
{
    using value_type                   = Type;
    using expected_disposable_strategy = Strategy;

    auto subscribe(auto&& observer) const
    {
        observer.set_upstream(rpp::composite_disposable_wrapper::make());
    }
};

template<rpp::constraint::decayed_type Type, rpp::details::observables::constraint::disposable_strategy Strategy>
struct wrapped_observable_strategy_no_set_upstream
{
    using value_type                   = Type;
    using expected_disposable_strategy = Strategy;

    auto subscribe(auto&&) const {}
};

template<typename T>
void test_operator_over_observable_with_disposable(auto&& op)
{
    SECTION("operator disposes disposable")
    {
        auto                                    observable_disposable = rpp::composite_disposable_wrapper::make();
        std::optional<rpp::dynamic_observer<T>> saved_observer{};
        auto                                    observable = rpp::source::create<T>([&observable_disposable, &saved_observer](auto&& obs) {
            obs.set_upstream(observable_disposable);
            saved_observer.emplace(std::forward<decltype(obs)>(obs).as_dynamic());
        });

        auto observer_disposable = rpp::composite_disposable_wrapper::make();
        op(observable) | rpp::ops::subscribe(observer_disposable, [](const auto&) {});

        observer_disposable.dispose();
        CHECK(observable_disposable.is_disposed());
    }

    SECTION("set_upstream with fixed_disposable_strategy_selector<1>")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_set_upstream<T, rpp::details::observables::fixed_disposable_strategy_selector<1>>>{})
                          .subscribe([](const auto&) {}, rpp::utils::rethrow_error_t{}));
    }

    SECTION("set_upstream with dynamic_disposable_strategy_selector<0>")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_set_upstream<T, rpp::details::observables::dynamic_disposable_strategy_selector<0>>>{})
                          .subscribe([](const auto&) {}, rpp::utils::rethrow_error_t{}));
    }

    SECTION("none_disposable_strategy")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_no_set_upstream<T, rpp::details::observables::bool_disposable_strategy_selector>>{})
                          .subscribe([](const auto&) {}, rpp::utils::rethrow_error_t{}));
    }

    SECTION("fixed_disposable_strategy_selector<0>")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_no_set_upstream<T, rpp::details::observables::fixed_disposable_strategy_selector<0>>>{})
                          .subscribe([](const auto&) {}, rpp::utils::rethrow_error_t{}));
    }

    SECTION("dynamic_disposable_strategy_selector<0>")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_no_set_upstream<T, rpp::details::observables::dynamic_disposable_strategy_selector<0>>>{})
                          .subscribe([](const auto&) {}, rpp::utils::rethrow_error_t{}));
    }

    SECTION("set_upstream with none_disposable_strategy calls on_error")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_set_upstream<T, rpp::details::observables::bool_disposable_strategy_selector>>{})
                          .subscribe([](const auto&) {}, [](const std::exception_ptr& err) { CHECK_THROWS_AS(std::rethrow_exception(err), rpp::utils::more_disposables_than_expected); }));
    }

    SECTION("set_upstream with fixed_disposable_strategy_selector<0> calls on_error")
    {
        CHECK_NOTHROW(op(rpp::observable<T, wrapped_observable_strategy_set_upstream<T, rpp::details::observables::fixed_disposable_strategy_selector<0>>>{})
                          .subscribe([](const auto&) {}, [](const std::exception_ptr& err) { CHECK_THROWS_AS(std::rethrow_exception(err), rpp::utils::more_disposables_than_expected); }));
    }
}

template<typename T>
void test_operator_with_disposable(auto&& op)
{
    test_operator_over_observable_with_disposable<T>([op](auto&& observable) { return observable | op; });
}
