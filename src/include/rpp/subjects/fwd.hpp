//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/subscriptions/fwd.hpp>

namespace rpp::subjects::details
{
struct subject_tag;

template<typename Strategy, typename T>
concept subject_strategy = std::constructible_from<Strategy, rpp::composite_subscription> && requires(Strategy t)
{
    {t.get_subscriber()} -> rpp::constraint::subscriber;
    t.on_subscribe(std::declval<rpp::dynamic_subscriber<T>>());
};

template<rpp::constraint::decayed_type T, subject_strategy<T> Strategy>
class base_subject;
} // namespace rpp::subjects::details

namespace rpp::subjects
{
template<rpp::constraint::decayed_type T>
class publish_subject;
} // namespace rpp::subjects
