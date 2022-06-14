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

#include <rpp/observables/fwd.hpp>

#include <type_traits>

namespace rpp::utils
{
namespace details
{
    template<typename T>
    struct extract_observable_type
    {
        template<typename TT>
        static TT deduce(const virtual_observable<TT>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };
} // namespace details

template<typename T>
using extract_observable_type_t = typename details::extract_observable_type<T>::type;
} // namespace rpp::utils