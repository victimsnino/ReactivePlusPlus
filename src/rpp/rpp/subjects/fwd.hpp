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
namespace details
{
    template<typename TSubject>
    struct extract_subject_type 
    {
        template<typename TT, typename Strategy>
        constexpr static TT deduce(const rpp::subjects::details::base_subject<TT, Strategy>&);

        using type = decltype(deduce(std::declval<std::decay_t<TSubject>>()));
    };

    template<typename TSubject>
    struct is_subject_t
    {
        template<typename TT, typename Strategy>
        constexpr static std::true_type  deduce(const rpp::subjects::details::base_subject<TT, Strategy>&);
        constexpr static std::false_type deduce(...);

        using type = decltype(deduce(std::declval<std::decay_t<TSubject>*>()));
    };

} // namespace details

template<typename T>
using extract_subject_type_t = typename details::extract_subject_type<std::decay_t<T>>::type;
} // namespace rpp::utils

namespace rpp::constraint
{
template<typename T>
concept subject = rpp::subjects::utils::details::is_subject_t<std::decay_t<T>>::type::value;
}
