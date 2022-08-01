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

#include <rpp/observers/fwd.hpp>

#include <type_traits>

namespace rpp::details
{
struct subscriber_tag;
} // namespace rpp::details

namespace rpp::constraint
{
template<typename T> concept observer_callbacks_exists = requires(const T t)
{
    // t.on_next(...);
    t.on_error(std::declval<std::exception_ptr>());
    t.on_completed();
};

template<typename T> concept observer = std::is_base_of_v<details::observer_tag, std::decay_t<T>> && !std::is_base_of_v<details::subscriber_tag, std::decay_t<T>> && observer_callbacks_exists<T>;

template<typename T> concept decayed_observer                = observer<T> && decayed_type<T>;
}

namespace rpp::utils
{
namespace details
{
    template<rpp::constraint::observer T>
    struct extract_observer_type
    {
        template<typename TT>
        static TT deduce(const interface_observer<TT>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };

} // namespace details
template<rpp::constraint::observer T>
using extract_observer_type_t = typename details::extract_observer_type<T>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T, typename Type> concept observer_on_next_exists = requires(const T t)
{
    t.on_next(std::declval<Type>());
};

template<typename T, typename Type> concept observer_of_type = observer<T> && std::same_as<utils::extract_observer_type_t<T>, Type> && observer_on_next_exists<T, Type>;
} // namespace rpp::constraint
