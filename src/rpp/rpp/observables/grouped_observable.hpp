//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/observable.hpp>

namespace rpp
{
template<constraint::decayed_type KeyType, constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class grouped_observable final : public observable<Type, Strategy>
{
public:
    grouped_observable(KeyType key, const Strategy& strategy)
        : observable<Type, Strategy>{strategy}
        , m_key{std::move(key)}
    {
    }

    grouped_observable(KeyType key, Strategy&& strategy)
        : observable<Type, Strategy>{std::move(strategy)}
        , m_key{std::move(key)}
    {
    }

    const KeyType& get_key() const { return m_key; }

private:
    KeyType m_key;
};
}