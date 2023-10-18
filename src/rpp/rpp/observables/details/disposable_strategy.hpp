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
enum class AtomicMode
{
    NonAtomic = 0,
    Atomic = 1
};

template<AtomicMode Mode>
using deduce_atomic_bool = std::conditional_t<Mode == AtomicMode::Atomic, observers::atomic_bool, observers::non_atomic_bool>;

template<size_t PreallocatedCount = 0, AtomicMode Mode = AtomicMode::NonAtomic>
struct dynamic_disposable_strategy_selector
{
    template<size_t Count>
    using add = dynamic_disposable_strategy_selector<PreallocatedCount+Count, Mode>;

    using disposable_container = disposables::dynamic_disposables_container<PreallocatedCount>;
    using disposable_strategy  = observers::dynamic_local_disposable_strategy<PreallocatedCount, deduce_atomic_bool<Mode>>;
};

template<size_t Count>
using atomic_dynamic_disposable_strategy_selector = dynamic_disposable_strategy_selector<Count, AtomicMode::Atomic>;

struct default_disposable_strategy_selector
{
    template<size_t Count>
    using add = default_disposable_strategy_selector;

    using disposable_container = dynamic_disposable_strategy_selector<0, AtomicMode::Atomic>::disposable_container;
    using disposable_strategy = dynamic_disposable_strategy_selector<0, AtomicMode::Atomic>::disposable_strategy;
};

template<size_t Count, AtomicMode Mode = AtomicMode::NonAtomic>
struct fixed_disposable_strategy_selector
{
    template<size_t AddCount>
    using add = fixed_disposable_strategy_selector<Count+AddCount, Mode>;

    using disposable_container = disposables::static_disposables_container<Count>;
    using disposable_strategy = observers::static_local_disposable_strategy<Count, deduce_atomic_bool<Mode>>;
};

template<AtomicMode Mode>
struct fixed_disposable_strategy_selector<0, Mode>
{
    template<size_t Count>
    using add = fixed_disposable_strategy_selector<Count, Mode>;

    using disposable_container = default_disposable_strategy_selector::disposable_container;
    using disposable_strategy = observers::bool_local_disposable_strategy<deduce_atomic_bool<Mode>>;
};

template<size_t Count>
using atomic_fixed_disposable_strategy_selector = fixed_disposable_strategy_selector<Count, AtomicMode::Atomic>;

template<AtomicMode Mode = AtomicMode::NonAtomic>
using bool_disposable_strategy_selector = fixed_disposable_strategy_selector<0, Mode>;

using atomic_bool_disposable_strategy_selector = bool_disposable_strategy_selector<AtomicMode::Atomic>;



namespace details
{
    template<typename T>
    auto* deduce_disposable_strategy()
    {
        if constexpr (requires { typename T::expected_disposable_strategy; })
            return static_cast<typename T::expected_disposable_strategy*>(nullptr);
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
    typename T::disposable_strategy;
    typename T::disposable_container;
    requires observers::constraint::disposable_strategy<typename T::disposable_strategy>;
};
}
}