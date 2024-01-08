//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/disposables/fwd.hpp>
#include <rpp/observers/fwd.hpp>

#include <rpp/utils/constraints.hpp>

#include <rpp/utils/utils.hpp>

namespace rpp::subjects::details
{
namespace constraint
{
template<typename Strategy, typename T>
concept subject_strategy = requires(Strategy t, rpp::details::fake_observer<T>&& obs)
{
    {t.get_observer()} -> rpp::constraint::observer;
    t.on_subscribe(std::move(obs));
    {t.get_disposable() } -> rpp::constraint::decayed_same_as<rpp::disposable_wrapper>;
};
}
template<rpp::constraint::decayed_type T, constraint::subject_strategy<T> Strategy>
class base_subject;

template<rpp::constraint::decayed_type Type>
class publish_strategy;

template<rpp::constraint::decayed_type Type>
class serialized_strategy;
}

namespace rpp::subjects
{
template<rpp::constraint::decayed_type Type>
class publish_subject;

template<rpp::constraint::decayed_type Type>
class serialized_subject;
}

namespace rpp::subjects::utils
{
template<typename T>
using extract_subject_type_t = typename rpp::utils::extract_base_type_params_t<T, rpp::subjects::details::base_subject>::template type_at_index_t<0>;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T>
concept subject = rpp::utils::is_base_of_v<T, rpp::subjects::details::base_subject>;
}
