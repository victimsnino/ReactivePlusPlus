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

#include <rpp/defs.hpp>
#include <rpp/observables/specific_observable.hpp> // base
#include <rpp/utils/operator_declaration.hpp>      // for header include

#include <memory>

IMPLEMENTATION_FILE(dynamic_observable_tag);

namespace rpp::details
{
template<constraint::decayed_type Type>
class dynamic_observable_state
{
public:
    template<constraint::observable_of_type<Type> TObs>
    dynamic_observable_state(TObs&& obs)
        : m_impl{std::make_shared<dynamic_observable_state_impl<std::decay_t<TObs>>>(std::forward<TObs>(obs))} {}

    template<constraint::on_subscribe_fn<Type> TOnSub>
        requires (!constraint::decayed_same_as<TOnSub, dynamic_observable_state<Type>>)
    dynamic_observable_state(TOnSub&& on_sub)
        : m_impl{ std::make_shared<dynamic_observable_state_impl<specific_observable<Type, std::decay_t<TOnSub>>>>(std::forward<TOnSub>(on_sub)) } {}

    dynamic_observable_state(const dynamic_observable_state& other)                = default;
    dynamic_observable_state(dynamic_observable_state&& other) noexcept            = default;
    dynamic_observable_state& operator=(const dynamic_observable_state& other)     = default;
    dynamic_observable_state& operator=(dynamic_observable_state&& other) noexcept = default;

    composite_subscription operator()(const dynamic_subscriber<Type>& subscriber) const
    {
        return (*m_impl)(subscriber);
    }
private:
    struct interface_dynamic_observable_state_impl
    {
        virtual ~interface_dynamic_observable_state_impl() = default;

        virtual composite_subscription operator()(const dynamic_subscriber<Type>& subscriber) const = 0;
    };

    template<constraint::observable TObs>
    class dynamic_observable_state_impl final : public interface_dynamic_observable_state_impl
    {
    public:
        dynamic_observable_state_impl(TObs&& observable)
            : m_observable{std::move(observable)} {}

        dynamic_observable_state_impl(const TObs& observable)
            : m_observable{observable} {}

        composite_subscription operator()(const dynamic_subscriber<Type>& subscriber) const override
        {
            return m_observable.subscribe(subscriber);
        }

    private:
        RPP_NO_UNIQUE_ADDRESS TObs m_observable{};
    };

    std::shared_ptr<interface_dynamic_observable_state_impl> m_impl{};
};
} // namespace rpp::details

namespace rpp
{
/**
 * \brief Type-less observable (or partially untyped) that has the notion of Type but hides the notion of on_subscribe<Type> for C++ compiler.
 *
 * \details This is a C++ technique called type-erasure. Multiple instances of the observable<type> that may have different upstream graphs are considered homogeneous. i.e. They can be stored in the same container, e.g. std::vector.
 * As a result, it uses heap to store on_subscribe and hide its type.
 *
 * \param Type is the value type. Observable of type means this source could emit a sequence of items of that "Type".
 * \ingroup observables
 */
template<constraint::decayed_type Type>
class dynamic_observable : public specific_observable<Type, details::dynamic_observable_state<Type>>
{
public:
    using base = specific_observable<Type, details::dynamic_observable_state<Type>>;
    using base::base;

    explicit dynamic_observable(constraint::on_subscribe_fn<Type> auto&& on_subscribe)
        : base{details::dynamic_observable_state<Type>{std::forward<decltype(on_subscribe)>(on_subscribe)}} {}

    template<constraint::observable_of_type<Type> TObs>
        requires (!std::is_same_v<std::decay_t<TObs>, dynamic_observable<Type>>)
    dynamic_observable(TObs&& observable)
        : base{details::dynamic_observable_state<Type>{std::forward<TObs>(observable)}} {}
};

template<constraint::observable TObs>
dynamic_observable(TObs obs) -> dynamic_observable<utils::extract_observable_type_t<TObs>>;

template<typename OnSub>
dynamic_observable(OnSub on_subscribe) -> dynamic_observable<utils::extract_subscriber_type_t<utils::function_argument_t<OnSub>>>;
} // namespace rpp
