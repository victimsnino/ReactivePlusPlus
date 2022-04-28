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
    template<typename T, typename Strategy>
    T extract_subject_type(const subjects::details::base_subject<T, Strategy>&);
} // namespace details

template<typename T>
using extract_subject_type_t = decltype(details::extract_subject_type(std::declval<std::decay_t<T>>()));
} // namespace rpp::subjects::utils
