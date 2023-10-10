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


#include <type_traits>

namespace rpp::details::observers
{
using external_disposable_strategy = composite_disposable_wrapper;
struct none_disposable_strategy;
class local_disposable_strategy;

namespace details
{
    template<typename T>
    auto* deduce_disposable_strategy()
    {
        if constexpr (requires { typename T::DisposableStrategyToUseWithThis; })
            return static_cast<typename T::DisposableStrategyToUseWithThis*>(nullptr);
        else
            return static_cast<local_disposable_strategy*>(nullptr);
    }
}
template<typename T>
using deduce_disposable_strategy_t = std::remove_pointer_t<decltype(details::deduce_disposable_strategy<T>())>;
}