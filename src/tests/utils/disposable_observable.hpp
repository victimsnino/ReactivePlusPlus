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

#include <rpp/sources/create.hpp>
#include <rpp/disposables/composite_disposable.hpp>

#include <snitch/snitch.hpp>

template<typename T>
auto observable_with_disposable(rpp::disposable_wrapper d)
{
    return rpp::source::create<T>([d](auto&& obs)
    {
        obs.set_upstream(d);
    });
}

template<typename T>
void test_operator_over_observable_with_disposable(auto&& op)
{
    auto observable_disposable = std::make_shared<rpp::composite_disposable>();
    auto observable = observable_with_disposable<T>(observable_disposable);

    auto observer_disposable = std::make_shared<rpp::composite_disposable>();
    op(observable) | rpp::ops::subscribe(rpp::composite_disposable_wrapper{observer_disposable}, [](const auto&){});

    observer_disposable->dispose();
    CHECK(observable_disposable->is_disposed());
}

template<typename T>
void test_operator_with_disposable(auto&& op)
{
    test_operator_over_observable_with_disposable<T>([op](auto&& observable){return observable | op; });
}
