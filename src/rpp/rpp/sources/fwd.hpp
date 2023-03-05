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
#include <rpp/memory_model.hpp>

namespace rpp::source
{
template<memory_model memory_model = memory_model::use_stack/*, schedulers::constraint::scheduler TScheduler = schedulers::trampoline*/>
auto from_iterable(constraint::iterable auto&& iterable/*, const TScheduler& scheduler = TScheduler{}*/);// requires rpp::details::is_header_included <rpp::details::from_tag, TScheduler > ;
} // namespace rpp::sopurce

