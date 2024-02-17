//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/fwd.hpp>

namespace rpp::subjects::details
{
    namespace constraint
    {
        template<typename Strategy, typename T>
        concept subject_on_subscribe = requires(const Strategy& t, rpp::details::observers::fake_observer<T>&& obs) {
            t.on_subscribe(std::move(obs));
        };
    } // namespace constraint

    template<rpp::constraint::decayed_type T, constraint::subject_on_subscribe<T> Strategy>
    struct subject_on_subscribe
    {
        using value_type                   = T;
        using expected_disposable_strategy = rpp::details::observables::deduce_disposable_strategy_t<Strategy>;

        Strategy strategy;

        template<rpp::constraint::observer_of_type<T> TObs>
        void subscribe(TObs&& sub) const
        {
            strategy.on_subscribe(std::forward<TObs>(sub));
        }
    };
} // namespace rpp::subjects::details
