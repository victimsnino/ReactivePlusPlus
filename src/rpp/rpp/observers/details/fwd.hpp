//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/disposables/fwd.hpp>

#include <rpp/disposables/details/container.hpp>

#include <type_traits>

namespace rpp::details::observers
{
    class atomic_bool;
    class non_atomic_bool;

    template<typename DisposableContainer, rpp::constraint::any_of<atomic_bool, non_atomic_bool> Bool>
    class local_disposable_strategy;

    /**
     * @brief No any disposable logic at all. Used only inside proxy-forwarding operators where extra disposable logic not requires
     */
    struct none_disposable_strategy;

    /**
     * @brief Dynamic disposable logic based on pre-allocated vector
     */
    template<size_t Count, rpp::constraint::any_of<atomic_bool, non_atomic_bool> Bool>
    using dynamic_local_disposable_strategy = local_disposable_strategy<disposables::dynamic_disposables_container<Count>, Bool>;

    /**
     * @brief Same as dynamic strategy, but based on array.
     */
    template<size_t Count, rpp::constraint::any_of<atomic_bool, non_atomic_bool> Bool>
    using static_local_disposable_strategy = local_disposable_strategy<disposables::static_disposables_container<Count>, Bool>;

    /**
     * @brief Just an boolean with no any disposables
     */
    template<rpp::constraint::any_of<atomic_bool, non_atomic_bool> Bool>
    using bool_local_disposable_strategy = local_disposable_strategy<disposables::none_disposables_container, Bool>;

    /**
     * @brief External disposable used as strategy
     */
    using external_disposable_strategy = composite_disposable_wrapper;

    namespace constraint
    {
        template<typename T>
        concept disposable_strategy = requires(T& v, const T& const_v, const disposable_wrapper& d) {
            v.add(d);
            {
                const_v.is_disposed()
            } -> std::same_as<bool>;
            const_v.dispose();
        };
    } // namespace constraint

    template<typename T>
    concept has_disposable_strategy = requires { typename T::preferred_disposable_strategy; };

    namespace details
    {
        template<typename T>
        auto* deduce_disposable_strategy()
        {
            if constexpr (has_disposable_strategy<T>)
                return static_cast<typename T::preferred_disposable_strategy*>(nullptr);
            else
                return static_cast<dynamic_local_disposable_strategy<0, atomic_bool>*>(nullptr);
        }
    } // namespace details

    template<typename T>
    using deduce_disposable_strategy_t = std::remove_pointer_t<decltype(details::deduce_disposable_strategy<T>())>;
} // namespace rpp::details::observers
