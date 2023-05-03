//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/sources/from.hpp>
#include <rpp/observables/base_observable.hpp>
#include <rpp/disposables/base_disposable.hpp>

#include <utility>

namespace rpp::details
{
template<rpp::constraint::observable TObservable, rpp::constraint::observable ...TObservables>
auto pack_observables(TObservable&& obs, TObservables&&...others)
{
    if constexpr ((rpp::constraint::decayed_same_as<TObservable, TObservables> && ...))
    {
        return pack_variadic(std::forward<TObservable>(obs), std::forward<TObservables>(others)...);
    }
    else
    {
        return pack_variadic(std::forward<TObservable>(obs).as_dynamic(), std::forward<TObservables>(others).as_dynamic()...);
    }
}

template<constraint::decayed_type PackedContainer, constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
void concat_source_drain(PackedContainer&& container, base_observer<Type, Strategy>&& observer);

template<constraint::decayed_type PackedContainer, constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
struct concat_source_observer_strategy
{
    mutable base_observer<Type, Strategy>         observer;
    rpp::base_disposable                          disposable;
    RPP_NO_UNIQUE_ADDRESS mutable PackedContainer container;

    void set_upstream(const disposable_wrapper& d)     { disposable.add(d.get_original()); }
    bool is_disposed() const                           { return observer.is_disposed(); }

    void on_next(const Type& v) const                  { observer.on_next(v); }
    void on_next(Type&& v) const                       { observer.on_next(std::move(v)); }
    void on_error(const std::exception_ptr& err) const { observer.on_error(err); }

    void on_completed() const
    {
        if (const auto itr = container.get_actual_iterator(); itr != std::cend(container))
        {
            container.increment_iterator();
            itr->subscribe(base_observer<Type, concat_source_observer_strategy<PackedContainer, Type, Strategy>>{std::move(observer), std::move(container)});
        }
    }
};

template<constraint::decayed_type PackedContainer, constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
void concat_source_drain(PackedContainer&& container, rpp::base_disposable&& disposable, base_observer<Type, Strategy>&& observer)
{
    if (const auto itr = container.get_actual_iterator(); itr != std::cend(container))
    {
        container.increment_iterator();
        itr->subscribe(base_observer<Type, concat_source_observer_strategy<PackedContainer, Type, Strategy>>{std::move(observer), std::move(disposable), std::forward<PackedContainer>(container)});
    } 
    else 
    {
        observer.on_completed();
    }
}

template<constraint::decayed_type PackedContainer>
struct concat_strategy
{
    RPP_NO_UNIQUE_ADDRESS PackedContainer container;

    using Type = utils::extract_observable_type_t<utils::iterable_value_t<PackedContainer>>;

    template<constraint::observer_strategy<Type> Strategy>
    void subscribe(base_observer<Type, Strategy>&& observer) const
    {
        rpp::base_disposable d{};
        observer.set_upstream(d);
        concat_source_drain(PackedContainer{container}, std::move(d), std::move(observer));
    }
};

template<typename PackedContainer>
auto make_concat_from_iterable(PackedContainer&& container)
{
    return base_observable<utils::iterable_value_t<std::decay_t<PackedContainer>>,
                           concat_strategy<std::decay_t<PackedContainer>>>{std::forward<PackedContainer>(container)};
}
}
namespace rpp::source
{
template<rpp::constraint::observable TObservable, rpp::constraint::observable ...TObservables>
    requires (std::same_as<rpp::utils::extract_observable_type_t<TObservable>, rpp::utils::extract_observable_type_t<TObservables>> && ...)
auto concat(TObservable&& obs, TObservables&&...others)
{
    return make_concat_from_iterable(pack_observables(std::forward<TObservable>(obs), std::forward<TObservables>(others)...));
}
}