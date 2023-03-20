//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/interface_observable.hpp>

IMPLEMENTATION_FILE(create_tag);

namespace rpp::source
{
template<constraint::decayed_type Type, std::invocable<rpp::interface_observer<Type>&&> OnSubscribeFn>
class create_observabe final : public interface_observable<Type>
{
public:
    create_observabe(const OnSubscribeFn& on_subscribe)
        : m_on_subscribe{on_subscribe} {}

    create_observabe(OnSubscribeFn&& on_subscribe)
        : m_on_subscribe{std::move(on_subscribe)} {}

protected:
    void subscribe_impl(interface_observer<Type>&& observer) const noexcept override
    {
        m_on_subscribe(std::move(observer));
    }

private:
    OnSubscribeFn m_on_subscribe;
};

template<constraint::decayed_type Type, std::invocable<rpp::interface_observer<Type>> OnSubscribeFn>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>
{
    using CreatedOnSubscribeFn = std::decay_t<OnSubscribeFn>;
    return create_observabe<Type, CreatedOnSubscribeFn>{std::forward<OnSubscribeFn>(on_subscribe)};
}

template<utils::is_callable OnSubscribeFn, constraint::decayed_type Type>
    requires std::invocable<OnSubscribeFn, rpp::interface_observer<Type>>
auto create(OnSubscribeFn&& on_subscribe) requires rpp::details::is_header_included<rpp::details::create_tag, Type, OnSubscribeFn>
{
    return create<Type>(std::forward<OnSubscribeFn>(on_subscribe));
}
} // namespace rpp::source
