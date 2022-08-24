//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//                             TC Wang 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/details/member_overload.hpp>
#include <rpp/observables/constraints.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/function_traits.hpp>


#include <tuple>

namespace rpp::details
{
struct combine_latest_tag;
}

namespace rpp::details
{

template<constraint::decayed_type Type, typename TCombiner, constraint::observable ...TOtherObservable>
struct combine_latest_impl;

template<constraint::decayed_type Type, typename SpecificObservable>
struct member_overload<Type, SpecificObservable, combine_latest_tag>
{

    /**
     * \brief Combines latest emissions from current observable and other observables when any of them emits.
     * \warning According to observable contract (https://reactivex.io/documentation/contract.html) emissions from any observable should be serialized, so, resulting observable uses mutex to satisfy this requirement
     *
     * \marble combine_latest_custom_combiner
       {
           source observable                                 : +---1    --    --    -2    --    -3    -|
           source other_observable                           : +-5--    -6    -7    --    -8    -     -|
           operator "combine_latest: x,y =>std::pair{x,y}"   : +---{1,5}-{1,6}-{1,7}-{2,7}-{2,8}-{3,8}-|
       }
     *
     * \param combiner combines emissions from all the observables using custom composition.
     * \param observables are observables whose emissions would be combined with the current observable's emissions
     * \return new specific_observable with the combine_latest operator as most recent operator.
     * \warning #include <rpp/operators/combine_latest.hpp>
     *
     * \par Examples
     * \snippet combine_latest.cpp combine_latest custom combiner
     *
     * \ingroup combining_operators
     * \see https://reactivex.io/documentation/operators/combinelatest.html
     */
    template<constraint::observable ...TOtherObservable, std::invocable<Type, utils::extract_observable_type_t<TOtherObservable>...> TCombiner>
    auto combine_latest(TCombiner&& combiner, TOtherObservable&&...observables) const& requires is_header_included<combine_latest_tag, TOtherObservable...>
    {
        using DownstreamType = utils::decayed_invoke_result_t<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>;

        return cast_this()->template lift<DownstreamType>(
            combine_latest_impl<Type, std::decay_t<TCombiner>, std::decay_t<TOtherObservable>...>{
                std::forward<TCombiner>(combiner),
                std::tuple{std::forward<TOtherObservable>(observables)...}
            });
    }

    template<constraint::observable ...TOtherObservable, std::invocable<Type, utils::extract_observable_type_t<TOtherObservable>...> TCombiner>
    auto combine_latest(TCombiner&& combiner, TOtherObservable&&...observables) && requires is_header_included<combine_latest_tag, TOtherObservable...>
    {
        using DownstreamType = utils::decayed_invoke_result_t<TCombiner, Type, utils::extract_observable_type_t<TOtherObservable>...>;

        return move_this().template lift<DownstreamType>(
            combine_latest_impl<Type, std::decay_t<TCombiner>, std::decay_t<TOtherObservable>...>{
                std::forward<TCombiner>(combiner),
                std::tuple{std::forward<TOtherObservable>(observables)...}
            });
    }

    /**
     * \brief Combines latest emissions from current observable and other observables when any of them emits. The combining result is std::tuple<...>.
     *
     * \marble combine_latest
       {
           source observable                : +---1    --    --    -2    --    -3    -|
           source other_observable          : +-5--    -6    -7    --    -8    -     -|
           operator "combine_latest:tuple"  : +---{1,5}-{1,6}-{1,7}-{2,7}-{2,8}-{3,8}-|
       }
     *
     * \param observables are observables whose emissions would be combined with the current observable's emissions
     * \return new specific_observable with the combine_latest operator as most recent operator.
     * \warning #include <rpp/operators/combine_latest.hpp>
     *
     * \par Examples
     * \snippet combine_latest.cpp combine_latest custom combiner
     *
     * \ingroup combining_operators
     * \see https://reactivex.io/documentation/operators/combinelatest.html
     */
    template<constraint::observable ...TOtherObservable>
    auto combine_latest(TOtherObservable&&...observables) const& requires is_header_included<combine_latest_tag, TOtherObservable...>
    {
        return cast_this()->combine_latest(utils::pack_to_tuple{}, std::forward<TOtherObservable>(observables)...);
    }

    template<constraint::observable ...TOtherObservable>
    auto combine_latest(TOtherObservable&&...observables) && requires is_header_included<combine_latest_tag, TOtherObservable...>
    {
        return move_this().combine_latest(utils::pack_to_tuple{}, std::forward<TOtherObservable>(observables)...);
    }

private:
    const SpecificObservable* cast_this() const
    {
        return static_cast<const SpecificObservable*>(this);
    }

    SpecificObservable&& move_this()
    {
        return std::move(*static_cast<SpecificObservable*>(this));
    }
};

} // namespace rpp::details
