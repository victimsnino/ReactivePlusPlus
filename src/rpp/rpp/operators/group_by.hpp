#pragma once

#include <rpp/operators/fwd/group_by.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/utilities.hpp>

#include <rpp/subjects/publish_subject.hpp>
#include <rpp/observables/grouped_observable.hpp>

#include <rpp/defs.hpp>

#include <map>

IMPLEMENTATION_FILE(group_by_tag);

namespace rpp::details
{
class group_by_state_base : public std::enable_shared_from_this<group_by_state_base>
{
public:
    group_by_state_base() = default;

    virtual ~group_by_state_base() noexcept = default;
    
    void on_subscribe(const composite_subscription& dest)
    {
        ++subscribers;
        dest.add([state = this->shared_from_this()]
        {
            if (--state->subscribers == 0)
                state->lifetime.unsubscribe();
        });
    }

    const auto& get_source_lifetime() const { return lifetime; }

private:
    composite_subscription lifetime{};
    std::atomic_size_t     subscribers{};
};

template<constraint::decayed_type           TKey,
         constraint::decayed_type           Type,
         std::strict_weak_order<TKey, TKey> KeyComparator>
struct group_by_state final : group_by_state_base
{
    group_by_state(const KeyComparator& comparator)
        : group_by_state_base{}
        , key_to_subject{comparator} {}

    std::map<TKey, subjects::publish_subject<Type>, KeyComparator> key_to_subject;
};

template<constraint::decayed_type Type>
struct group_by_on_subscribe
{
    subjects::publish_subject<Type>      subject;
    std::shared_ptr<group_by_state_base> state;

    void operator()(auto&& subscriber) const
    {
        state->on_subscribe(subscriber.get_subscription());
        subject.get_observable().subscribe(std::forward<decltype(subscriber)>(subscriber));
    }
};
} // namespace rpp::details

namespace rpp
{
template<constraint::decayed_type TKey, constraint::decayed_type ResValue>
using grouped_observable_group_by = grouped_observable<TKey, ResValue, details::group_by_on_subscribe<ResValue>>;
}

namespace rpp::details
{
template<constraint::decayed_type           Type,
         constraint::decayed_type           TKey,
         std::invocable<Type>               KeySelector,
         std::invocable<Type>               ValueSelector,
         std::strict_weak_order<TKey, TKey> KeyComparator>
struct group_by_lift_impl
{
    using ValueType = utils::decayed_invoke_result_t<ValueSelector, Type>;
    using StateType = group_by_state<TKey, utils::decayed_invoke_result_t<ValueSelector, Type>, KeyComparator>;

    RPP_NO_UNIQUE_ADDRESS KeySelector   key_selector;
    RPP_NO_UNIQUE_ADDRESS ValueSelector value_selector;
    RPP_NO_UNIQUE_ADDRESS KeyComparator comparator;

    template<constraint::subscriber TSub>
    class group_by_observer final : public typed_observer<Type>
    {
    public:
        group_by_observer(const std::shared_ptr<StateType>& state,
                          const TSub&                       subscriber,
                          const KeySelector&                key_selector,
                          const ValueSelector&              value_selector)
            : state{state}
            , subscriber{subscriber}
            , key_selector{key_selector}
            , value_selector{value_selector} {}

        void on_next(auto&& v) const                       { on_next_impl(std::forward<decltype(v)>(v));                 }
        void on_error(const std::exception_ptr& err) const { broadcast([&err](const auto& sub) { sub.on_error(err); });  }
        void on_completed() const                          { broadcast([](const auto& sub)     { sub.on_completed(); }); }

    private:
        void on_next_impl(auto&& val) const
        {
            auto key = key_selector(utils::as_const(val));
            auto [itr, inserted] = state->key_to_subject.try_emplace(key);

            if (inserted)
            {
                subscriber.on_next(grouped_observable_group_by<TKey, ValueType>{key, group_by_on_subscribe<ValueType>{itr->second, state}});
            }

            const auto& subject_sub = itr->second.get_subscriber();
            if (subject_sub.is_subscribed())
                subject_sub.on_next(value_selector(std::forward<decltype(val)>(val)));
        }

        void broadcast(const auto& action) const
        {
            for (const auto& [_, subject] : state->key_to_subject)
                action(subject.get_subscriber());

            action(subscriber);
        }

        std::shared_ptr<StateType>          state;
        TSub                                subscriber;
        RPP_NO_UNIQUE_ADDRESS KeySelector   key_selector;
        RPP_NO_UNIQUE_ADDRESS ValueSelector value_selector;
    };


    template<constraint::subscriber TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto state = std::make_shared<StateType>(comparator);

        state->on_subscribe(subscriber.get_subscription());

        return specific_subscriber{state->get_source_lifetime(), group_by_observer<std::decay_t<TSub>>{state, std::forward<TSub>(subscriber), key_selector, value_selector}};
    }
};

template<constraint::decayed_type           Type,
         constraint::decayed_type           TKey,
         std::invocable<Type>               KeySelector,
         std::invocable<Type>               ValueSelector,
         std::strict_weak_order<TKey, TKey> KeyComparator>
auto group_by_impl(auto&&          observable,
                   KeySelector&&   key_selector,
                   ValueSelector&& value_selector,
                   KeyComparator&& comparator)
{
    using Res = grouped_observable_group_by<TKey, utils::decayed_invoke_result_t<ValueSelector, Type>>;
    using Lifter = group_by_lift_impl<Type, TKey, std::decay_t<KeySelector>, std::decay_t<ValueSelector>, std::decay_t<KeyComparator>>;

    return std::forward<decltype(observable)>(observable)
            .template lift<Res>(Lifter{std::forward<KeySelector>(key_selector),
                                       std::forward<ValueSelector>(value_selector),
                                       std::forward<KeyComparator>(comparator)});
}
} // namespace rpp::details
