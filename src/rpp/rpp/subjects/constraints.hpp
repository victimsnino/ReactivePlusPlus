//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subjects/fwd.hpp>
#include <rpp/subjects/type_traits.hpp>

#include <concepts>

namespace rpp::subjects::constraint
{
template<typename T>
concept subject = std::derived_from<std::decay_t<T>, details::subject_tag>;

template<typename T, typename Type>
concept subject_of_type = subject<T> && std::same_as<utils::extract_subject_type_t<T>, Type>;

} // namespace rpp::subjects::constraint