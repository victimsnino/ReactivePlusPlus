//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/base_observable.hpp>

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
struct create_strategy
{
    OnSubscribe subscribe;
};
}

namespace rpp
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
using create_observable = base_observable<Type, details::create_strategy<Type, OnSubscribe>>;
}

namespace rpp::source
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
auto create(OnSubscribe&& on_subscribe)
{
    return create_observable<Type, std::decay_t<OnSubscribe>>{std::forward<OnSubscribe>(on_subscribe)};
}
}