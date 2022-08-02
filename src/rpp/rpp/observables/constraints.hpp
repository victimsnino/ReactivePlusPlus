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

#include <concepts>

namespace rpp::constraint
{
template<typename T> concept observable = std::derived_from<std::decay_t<T>, details::observable_tag>;
}

namespace rpp::utils
{
namespace details
{
    template<rpp::constraint::observable T>
    struct extract_observable_type
    {
        template<typename TT>
        static TT deduce(const rpp::details::typed_observable_tag<TT>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };
} // namespace details

template<rpp::constraint::observable T>
using extract_observable_type_t = typename details::extract_observable_type<T>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T, typename Type> concept observable_of_type = observable<T> && std::same_as<utils::extract_observable_type_t<T>, Type>;
} // namespace rpp::constraint