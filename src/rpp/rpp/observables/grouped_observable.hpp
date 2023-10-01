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

namespace rpp
{
template<constraint::decayed_type KeyType, constraint::observable BaseObservable>
class grouped_observable final : public BaseObservable
{
public:

    grouped_observable(KeyType key, BaseObservable&& inner)
        : BaseObservable{std::move(inner)}
        , m_key{std::move(key)}
    {
    }

    grouped_observable(KeyType key, const BaseObservable& inner)
        : BaseObservable{inner}
        , m_key{std::move(key)}
    {
    }

    const KeyType& get_key() const { return m_key; }

private:
    KeyType m_key;
};
}