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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/start_with.hpp>

#include <rpp/operators/concat.hpp>
#include <rpp/sources/just.hpp>


IMPLEMENTATION_FILE (start_with_tag);

namespace rpp::details
{
template<rpp::memory_model memory_model, constraint::decayed_type Type, constraint::decayed_same_as<Type> ...TTypes, constraint::observable_of_type<Type> TObservable>
auto start_with_impl(TObservable&& observable, TTypes&& ...vals)
{
    return std::forward<TObservable>(observable).start_with(rpp::source::just<memory_model>(std::forward<TTypes>(vals)...));
}
} // namespace rpp::details
