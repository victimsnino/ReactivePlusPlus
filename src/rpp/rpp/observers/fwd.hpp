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

#include <rpp/utils/constraints.hpp>

#include <exception>

namespace rpp::constraint
{
template<typename Fn, typename Type> concept on_next_fn      = std::invocable<std::decay_t<Fn>, Type>;
template<typename Fn>                concept on_error_fn     = std::invocable<std::decay_t<Fn>, std::exception_ptr>;
template<typename Fn>                concept on_completed_fn = std::invocable<std::decay_t<Fn>>;
} // namespace rpp::constraint

namespace rpp::details
{
struct as_dynamic_constructor_tag{};
} // namespace rpp::details

namespace rpp
{
template<constraint::decayed_type Type>
struct interface_observer;

template<constraint::decayed_type     Type,
         constraint::on_next_fn<Type> OnNext,
         constraint::on_error_fn      OnError,
         constraint::on_completed_fn  OnCompleted>
class anonymous_observer;

template<constraint::decayed_type Type>
class dynamic_observer;
} // namespace rpp

namespace rpp::utils
{
namespace details
{
    template<typename T>
    struct extract_observer_type
    {
        template<typename TT>
        static TT deduce(const rpp::interface_observer<TT>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };

} // namespace details
template<typename T>
using extract_observer_type_t = typename details::extract_observer_type<T>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T, typename Type> concept observer_of_type = std::derived_from<T, interface_observer<Type>>;
} // namespace rpp::constraint
