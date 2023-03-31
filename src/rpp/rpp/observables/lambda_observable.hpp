//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>
#include <rpp/observables/base_observable.hpp>

namespace rpp::details::observable
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
struct lambda_strategy
{
    OnSubscribe subscribe;
};
}

namespace rpp
{
template<constraint::decayed_type Type, constraint::on_subscribe<Type> OnSubscribe>
auto make_lambda_observable(OnSubscribe&& on_subscribe) -> lambda_observable<Type, std::decay_t<OnSubscribe>>
{
    return lambda_observable<Type, std::decay_t<OnSubscribe>>{std::forward<OnSubscribe>(on_subscribe)};
}
}