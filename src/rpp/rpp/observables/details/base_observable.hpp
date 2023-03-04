//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/interface_observable.hpp>

namespace rpp::details
{
template<constraint::decayed_type Type>
class base_observable : public interface_observable
{
public:
    composite_disposable subscribe(const interface_observer<Type>& observer) final
    {
        return subscribe_impl(observer);
    }

protected:
    virtual composite_disposable subscribe_impl(const interface_observer<Type>& observer) final = 0;
};
} // namespace rpp::details