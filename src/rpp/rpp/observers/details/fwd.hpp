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
    enum class disposable_mode : uint8_t
    {
        // Let observer deduce disposable mode
        Auto = 0,
        // No any disposable logic for observer expected
        None = 1,
        // Use external (passed to constructor) composite_disposable_wrapper as disposable
        External = 2
    };

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

    template<typename DisposableContainer>
    class local_disposable_strategy;

    /**
     * @brief No any disposable logic at all. Used only inside proxy-forwarding operators where extra disposable logic not requires
     */
    struct none_disposable_strategy;

    /**
     * @brief Keep disposables inside dynamic_disposables_container container (based on std::vector)
     */
    using dynamic_disposable_strategy = local_disposable_strategy<disposables::dynamic_disposables_container>;

    /**
     * @brief Keep disposables inside static_disposables_container container (based on std::array)
     */
    template<size_t Count>
    using static_disposable_strategy = local_disposable_strategy<disposables::static_disposables_container<Count>>;

    namespace details
    {
        template<disposable_mode mode>
        consteval auto* deduce_optimal_disposable_strategy()
        {
            static_assert(mode == disposable_mode::Auto || mode == disposable_mode::None || mode == disposable_mode::External);

            if constexpr (mode == disposable_mode::Auto)
                return static_cast<dynamic_disposable_strategy*>(nullptr);
            else if constexpr (mode == disposable_mode::None)
                return static_cast<none_disposable_strategy*>(nullptr);
            else if constexpr (mode == disposable_mode::External)
                return static_cast<composite_disposable_wrapper*>(nullptr);
        }
    } // namespace details

    template<rpp::details::observers::disposable_mode Mode>
    using deduce_optimal_disposable_strategy_t = std::remove_pointer_t<decltype(details::deduce_optimal_disposable_strategy<Mode>())>;
} // namespace rpp::details::observers
