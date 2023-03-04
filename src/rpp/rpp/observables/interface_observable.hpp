//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>
#include <rpp/observers/fwd.hpp>
#include <rpp/disposables/fwd.hpp>

namespace rpp
{
template<constraint::decayed_type Type>
struct interface_observable
{
    virtual ~interface_observable() = default;

    virtual composite_disposable subscribe(const interface_observer<Type>& observer) = 0;
};
} // namespace rpp