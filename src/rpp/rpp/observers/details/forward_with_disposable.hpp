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

#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/observers/observer.hpp>

namespace rpp::details
{
template<rpp::constraint::observer TObserver>
struct forward_with_disposable
{
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;

    RPP_NO_UNIQUE_ADDRESS TObserver   observer;
    rpp::composite_disposable_wrapper disposable;

    template<typename T>
    void on_next(T&& v) const
    {
        observer.on_next(std::forward<T>(v));
    }
    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }
    void on_completed() const { observer.on_completed(); }

    bool is_disposed() const { return observer.is_disposed(); }
    void set_upstream(const disposable_wrapper& d) const { disposable.add(d); }
};
} // namespace rpp::details