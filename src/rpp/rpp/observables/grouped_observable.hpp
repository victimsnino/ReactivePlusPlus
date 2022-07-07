//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observables/specific_observable.hpp>

namespace rpp
{
template<constraint::decayed_type KeyType,
         constraint::decayed_type Type,
         constraint::on_subscribe_fn<Type> OnSubscribeFn>
class grouped_observable final : public specific_observable<Type, OnSubscribeFn>
{
public:
    grouped_observable(KeyType key, const OnSubscribeFn& on_subscribe)
        : specific_observable<Type, OnSubscribeFn>{on_subscribe}
        , m_key{std::move(key)} {}

    grouped_observable(KeyType key, OnSubscribeFn&& on_subscribe)
        : specific_observable<Type, OnSubscribeFn>{std::move(on_subscribe)}
        , m_key{std::move(key)} {}

    const KeyType& get_key() const { return m_key; }

private:
    KeyType m_key;
};
} // namespace rpp

