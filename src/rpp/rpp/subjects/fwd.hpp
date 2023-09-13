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
}

namespace rpp::subjects
{
/**
 * @brief Subject which just multicasts values to observers subscribed on it. It contains two parts: observer and observable at the same time.
 *
 * @details Each observer obtains only values which emitted after corresponding subscribe. on_error/on_completer/unsubscribe cached and provided to new observers if any
 *
 * @warning this subject is not synchronized/serialized! It means, that expected to call callbacks of observer in the serialized way to follow observable contract: "Observables must issue notifications to observers serially (not in parallel).". If you are not sure or need extra serialization, please, use serialized_subject.
 *
 * @tparam Type value provided by this subject
 *
 * @ingroup subjects
 * @see https://reactivex.io/documentation/subject.html
 */
template<rpp::constraint::decayed_type Type>
using publish_subject = details::base_subject<Type, details::publish_strategy<Type>>;
}

namespace rpp::subjects::utils
{
namespace details
{
    template<typename T>
    struct extract_subject_type : std::false_type
    {
    };

    template<typename TT, typename Strategy>
    struct extract_subject_type<rpp::subjects::details::base_subject<TT, Strategy>> : std::true_type
    {
        using type = TT;
    };

} // namespace details

template<typename T>
using extract_subject_type_t = typename details::extract_subject_type<std::decay_t<T>>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T>
concept subject = rpp::subjects::utils::details::extract_subject_type<std::decay_t<T>>::value;
}
