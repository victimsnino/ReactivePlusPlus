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

#include <rpp/subjects/fwd.hpp>

namespace rpp::subjects::utils
{
namespace details
{
    template<typename T>
    struct extract_subject_type
    {
        template<typename TT, typename Strategy>
        static TT deduce(const subjects::details::base_subject<TT, Strategy>&);

        using type = decltype(deduce(std::declval<std::decay_t<T>>()));
    };
} // namespace details

template<typename T>
using extract_subject_type_t = typename details::extract_subject_type<T>::type;
} // namespace rpp::subjects::utils
