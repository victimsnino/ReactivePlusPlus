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
#include <utility>

namespace rpp::utils
{
namespace details
{
    template<typename T>
    struct extract_subscriber_type
    {
        template<typename TT>
        static TT deduce(const rpp::details::subscriber_base<TT>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };
} // namespace details

template<typename T>
using extract_subscriber_type_t = typename details::extract_subscriber_type<T>::type;
} // namespace rpp::utils