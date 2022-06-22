#pragma once

#include <rpp/operators/fwd/group_by.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>

#include <rpp/subjects/publish_subject.hpp>
#include <rpp/observables/grouped_observable.hpp>

#include <map>

IMPLEMENTATION_FILE(group_by_tag);

namespace rpp::details
{

template<constraint::decayed_type  Type>
struct group_by_on_subscribe
{
    rpp::subjects::publish_subject<Type> subject;

    void operator()(auto&& subscriber) const
    {
        subject.get_observable().subscribe(subscriber);
    }
};

template<constraint::decayed_type  TKey,
         constraint::decayed_type  Type,
         std::relation<TKey, TKey> KeyComparator>
struct group_by_state
{
    group_by_state(const KeyComparator& comparator) : key_to_subject{comparator} {}

    std::map<TKey, subjects::publish_subject<Type>, KeyComparator> key_to_subject;
};

template<constraint::decayed_type  Type,
         constraint::decayed_type  TKey,
         std::invocable<Type>      KeySelector,
         std::invocable<Type>      ValueSelector,
         std::relation<TKey, TKey> KeyComparator>
struct group_by_lift_impl
{
    using ValueType = rpp::utils::decayed_invoke_result_t<ValueSelector, Type>;

    [[no_unique_address]] KeySelector   key_selector;
    [[no_unique_address]] ValueSelector value_selector;
    [[no_unique_address]] KeyComparator comparator;

    template<constraint::subscriber TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        auto state = std::make_shared<group_by_state<TKey, ValueType, KeyComparator>>(comparator);

        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::forward<TSub>(subscriber),
                                                  get_on_next(state),
                                                  get_on_error(state),
                                                  get_on_completed(state));
    }

private:
    auto get_on_next(const std::shared_ptr<group_by_state<TKey, ValueType, KeyComparator>>& state) const
    {
        return [state, value_selector=value_selector, key_selector=key_selector](auto&& val, const auto& subscriber)
        {
            auto key = key_selector(utils::as_const(val));
            auto [itr, inserted] = state->key_to_subject.try_emplace(key);
            if (inserted)
            {
                subscriber.on_next(rpp::grouped_observable<TKey, ValueType, group_by_on_subscribe<ValueType>>{key, group_by_on_subscribe<ValueType>{itr->second}});
            }

            itr->second.get_subscriber().on_next(value_selector(std::forward<decltype(val)>(val)));
        };
    }

    auto get_on_error(const std::shared_ptr<group_by_state<TKey, ValueType, KeyComparator>>& state) const
    {
        return [state](const std::exception_ptr& err, const auto& subscriber)
        {
            for(const auto& [_, subject] : state->key_to_subject)
                subject.get_subscriber().on_error(err);

            subscriber.on_error(err);
        };
    }

    auto get_on_completed(const std::shared_ptr<group_by_state<TKey, ValueType, KeyComparator>>& state) const
    {
        return [state](const auto& subscriber)
        {
            for(const auto& [_, subject] : state->key_to_subject)
                subject.get_subscriber().on_completed();

            subscriber.on_completed();
        };
    }
};

template<constraint::decayed_type  Type,
         constraint::decayed_type  TKey,
         std::invocable<Type>      KeySelector,
         std::invocable<Type>      ValueSelector,
         std::relation<TKey, TKey> KeyComparator>
auto group_by_impl(auto&& observable, KeySelector&& key_selector, ValueSelector&& value_selector, KeyComparator&& comparator)
{
    using ResValue = utils::decayed_invoke_result_t<ValueSelector, Type>;
    return std::forward<decltype(observable)>(observable).template lift<rpp::grouped_observable<TKey, ResValue, group_by_on_subscribe<ResValue>>>(group_by_lift_impl<Type, TKey, std::decay_t<KeySelector>, std::decay_t<ValueSelector>, std::decay_t<KeyComparator>>{std::forward<KeySelector>(key_selector), std::forward<ValueSelector>(value_selector), std::forward<KeyComparator>(comparator)});
}
} // namespace rpp::details
