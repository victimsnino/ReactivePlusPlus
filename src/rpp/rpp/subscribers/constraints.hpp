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

#include <rpp/subscribers/fwd.hpp>

#include <type_traits>

namespace rpp::constraint
{
template<typename T> concept subscriber = std::is_base_of_v<details::subscriber_tag, std::decay_t<T>>;

}

namespace rpp::utils
{
namespace details
{
    template<rpp::constraint::subscriber T>
    struct extract_subscriber_type
    {
        template<typename TT>
        static TT deduce(const rpp::details::subscriber_base<TT>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };
} // namespace details

template<rpp::constraint::subscriber T>
using extract_subscriber_type_t = typename details::extract_subscriber_type<T>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T, typename Type> concept subscriber_of_type = subscriber<T> && std::same_as<utils::extract_subscriber_type_t<T>, Type>;

} // namespace rpp::constraint