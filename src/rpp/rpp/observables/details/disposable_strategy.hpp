//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/fwd.hpp>

#include <rpp/utils/constraints.hpp>

namespace rpp::details::observables
{
    struct dynamic_disposable_strategy
    {
        template<size_t Count>
        using add = dynamic_disposable_strategy;

        using disposables_container        = disposables::dynamic_disposables_container;
        using observer_disposable_strategy = observers::dynamic_disposable_strategy;
    };

    template<size_t Count>
    struct fixed_disposable_strategy
    {
        template<size_t AddCount>
        using add = fixed_disposable_strategy<Count + AddCount>;

        using disposables_container        = disposables::static_disposables_container<Count>;
        using observer_disposable_strategy = observers::static_disposable_strategy<Count>;
    };

    using default_disposable_strategy = dynamic_disposable_strategy;
    namespace details
    {
        template<typename T>
        consteval auto* deduce_optimal_disposable_strategy()
        {
            if constexpr (requires { typename T::optimal_disposable_strategy; })
                return static_cast<typename T::optimal_disposable_strategy*>(nullptr);
            else
                return static_cast<default_disposable_strategy*>(nullptr);
        }
    } // namespace details

    template<typename T>
    using deduce_optimal_disposable_strategy_t = std::remove_pointer_t<decltype(details::deduce_optimal_disposable_strategy<T>())>;

    namespace constraint
    {
        template<typename T>
        concept disposable_strategy = requires(const T&) {
            typename T::template add<size_t{}>;
            typename T::observer_disposable_strategy;
            typename T::disposables_container;
            requires observers::constraint::disposable_strategy<typename T::observer_disposable_strategy>;
        };
    } // namespace constraint
} // namespace rpp::details::observables
