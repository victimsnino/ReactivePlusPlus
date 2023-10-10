//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/utils/constraints.hpp>
#include <rpp/observers/fwd.hpp>

namespace rpp::details::observables
{
struct default_disposable_strategy_selector
{
    template<size_t Count>
    using add = default_disposable_strategy_selector;

    using observer_disposable_strategy = observers::dynamic_local_disposable_strategy<0>;
};

template<size_t PreallocatedCount = 0>
struct dynamic_disposable_strategy_selector
{
    template<size_t Count>
    using add = dynamic_disposable_strategy_selector<PreallocatedCount+Count>;

    using observer_disposable_strategy = observers::dynamic_local_disposable_strategy<PreallocatedCount>;
};

template<size_t Count>
struct fixed_disposable_strategy_selector
{
    template<size_t AddCount>
    using add = fixed_disposable_strategy_selector<Count+AddCount>;

    using observer_disposable_strategy = observers::static_local_disposable_strategy<Count>;
};

struct none_disposable_strategy_selector
{
    template<size_t Count>
    using add = fixed_disposable_strategy_selector<Count>;

    using observer_disposable_strategy = observers::bool_local_disposable_strategy;
};

namespace details
{
    template<typename T>
    auto* deduce_disposable_strategy()
    {
        if constexpr (requires { typename T::DisposableStrategyToUseWithThis; })
            return static_cast<typename T::DisposableStrategyToUseWithThis*>(nullptr);
        else
            return static_cast<default_disposable_strategy_selector*>(nullptr);
    }
}

template<typename T>
using deduce_disposable_strategy_t = std::remove_pointer_t<decltype(details::deduce_disposable_strategy<T>())>;
namespace constraint
{
template<typename T>
concept disposable_strategy = requires(const T&)
{
    typename T::template add<size_t{}>;
    typename T::observer_disposable_strategy;
    observers::constraint::disposable_strategy<typename T::observer_disposable_strategy>;
};
}