//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include "rpp/disposables/fwd.hpp"
#include <rpp/operators/fwd.hpp>
#include <rpp/operators/details/strategy.hpp>
#include <rpp/observables/grouped_observable.hpp>
#include <rpp/subjects/publish_subject.hpp>
#include <rpp/disposables/refcount_disposable.hpp>

#include <rpp/utils/function_traits.hpp>
#include <map>
#include <type_traits>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type T>
struct group_by_observable_strategy;
}

namespace rpp
{
template<constraint::decayed_type TKey, constraint::decayed_type ResValue>
using grouped_observable_group_by = grouped_observable<TKey, ResValue, operators::details::group_by_observable_strategy<ResValue>>;
}

namespace rpp::operators::details
{
struct group_by_inner_observer_strategy
{
    std::shared_ptr<refcount_disposable> disposable;

    void set_upstream(const rpp::constraint::observer auto&, const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }
    
    constexpr static forwarding_on_next_strategy on_next{};
    constexpr static forwarding_on_error_strategy on_error{};
    constexpr static forwarding_on_completed_strategy on_completed{};
    constexpr static forwarding_is_disposed_strategy is_disposed{};
    constexpr static empty_on_subscribe on_subscribe{};

};

template<rpp::constraint::decayed_type T, rpp::constraint::decayed_type KeySelector, rpp::constraint::decayed_type ValueSelector, rpp::constraint::decayed_type KeyComparator>
struct group_by_observer_strategy
{
    using DisposableStrategyToUseWithThis = rpp::details::none_disposable_strategy;

    using TKey = utils::decayed_invoke_result_t<KeySelector, T>;
    using Type = utils::decayed_invoke_result_t<ValueSelector, T>;

    RPP_NO_UNIQUE_ADDRESS KeySelector   key_selector;
    RPP_NO_UNIQUE_ADDRESS ValueSelector value_selector;
    RPP_NO_UNIQUE_ADDRESS KeyComparator comparator;

    mutable std::map<TKey, subjects::publish_subject<Type>, KeyComparator> key_to_subject{};
    std::shared_ptr<refcount_disposable> disposable = std::make_shared<refcount_disposable>();

    void on_subscribe(rpp::constraint::observer auto& obs) const
    {
        obs.set_upstream(rpp::disposable_wrapper{disposable});
    }

    void set_upstream(const rpp::constraint::observer auto&, const rpp::disposable_wrapper& d) const
    {
        disposable->add(d.get_original());
    }

    bool is_disposed(const rpp::constraint::observer auto&) const
    {
        return disposable->is_disposed();
    }

    template<rpp::constraint::decayed_same_as<T> TT>
    void on_next(const rpp::constraint::observer auto& obs, TT&& val) const
    {
        const auto subject = deduce_subject(obs, val);
        if (!subject)
            return;

        const auto& subject_obs = subject->get_observer();
        if (!subject_obs.is_disposed())
            subject_obs.on_next(value_selector(std::forward<TT>(val)));
    }

    void on_error(const rpp::constraint::observer auto& obs, const std::exception_ptr& err) const
    {
        for (const auto& [_, subject] : key_to_subject)
            subject.get_observer().on_error(err);

        obs.on_error(err);
    }

    void on_completed(const rpp::constraint::observer auto& obs) const
    {
        for (const auto& [_, subject] : key_to_subject)
            subject.get_observer().on_completed();

        obs.on_completed();
    }

private:
    template<rpp::constraint::decayed_same_as<T> TT>
    const subjects::publish_subject<Type>* deduce_subject(const rpp::constraint::observer auto& obs, const TT& val) const
    {
        auto key = key_selector(utils::as_const(val));

        if (obs.is_disposed())
        {
            const auto itr = key_to_subject.find(key);
            return itr == key_to_subject.cend() ? nullptr : &itr->second;
        }

        auto [itr, inserted] = key_to_subject.try_emplace(key);

        if (inserted)
        {
            disposable->add(itr->second.get_disposable().get_original());
            obs.on_next(rpp::grouped_observable_group_by<TKey, Type>{std::move(key), group_by_observable_strategy<Type>{itr->second, disposable}});
        }

        return &itr->second;
    }
};

template<rpp::constraint::decayed_type T>
struct group_by_observable_strategy
{
    using ValueType = T;

    rpp::subjects::publish_subject<T> subj;
    std::shared_ptr<refcount_disposable> disposable;

    template<rpp::constraint::observer_strategy<T> Strategy>
    void subscribe(observer<T, Strategy>&& obs) const
    {
        obs.set_upstream(rpp::disposable_wrapper::from_weak(disposable->add_ref().get_original()));
        subj.get_observable()
            .subscribe(rpp::observer<T, operator_strategy_base<T, observer<T, Strategy>, group_by_inner_observer_strategy>>{std::move(obs), disposable});
    }
};

template<rpp::constraint::decayed_type KeySelector, rpp::constraint::decayed_type ValueSelector, rpp::constraint::decayed_type KeyComparator>
struct group_by_t : public operators::details::template_operator_observable_strategy<group_by_observer_strategy, KeySelector, ValueSelector, KeyComparator>
{
    using operators::details::template_operator_observable_strategy<group_by_observer_strategy, KeySelector, ValueSelector, KeyComparator>::template_operator_observable_strategy;

    template<rpp::constraint::decayed_type T>
        requires std::invocable<KeySelector, T> &&
                 std::invocable<ValueSelector, T> &&
                 std::strict_weak_order<KeyComparator, utils::decayed_invoke_result_t<KeySelector, T>, utils::decayed_invoke_result_t<KeySelector, T>>
    using ResultValue = grouped_observable<utils::decayed_invoke_result_t<KeySelector, T>, utils::decayed_invoke_result_t<ValueSelector, T>, group_by_observable_strategy<utils::decayed_invoke_result_t<ValueSelector, T>>>;
};
}
namespace rpp::operators
{
template<typename KeySelector,
         typename ValueSelector,
         typename KeyComparator>
    requires
    (
        (!utils::is_not_template_callable<KeySelector> || !std::same_as<void, std::invoke_result_t<KeySelector, utils::convertible_to_any>>) &&
        (!utils::is_not_template_callable<ValueSelector> || !std::same_as<void, std::invoke_result_t<ValueSelector, utils::convertible_to_any>>) &&
        (!utils::is_not_template_callable<KeyComparator> || std::strict_weak_order<KeyComparator, utils::convertible_to_any, utils::convertible_to_any>)
    )
auto group_by(KeySelector&& key_selector, ValueSelector&& value_selector, KeyComparator&& comparator)
{
    return details::group_by_t<std::decay_t<KeySelector>, std::decay_t<ValueSelector>, std::decay_t<KeyComparator>>
    {
        std::forward<KeySelector>(key_selector), std::forward<ValueSelector>(value_selector), std::forward<KeyComparator>(comparator)
    };
}
} // namespace rpp::operators