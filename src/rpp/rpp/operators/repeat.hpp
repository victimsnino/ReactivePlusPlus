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

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/repeat.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(repeat_tag);

namespace rpp::details
{
template<constraint::decayed_type Type, typename SpecificObservable, typename Predicate>
class repeat_on_completed
{
public:
    repeat_on_completed(std::shared_ptr<SpecificObservable> shared_observable,
                        Predicate                           predicate)
        : m_shared_observable{std::move(shared_observable)}
        , m_predicate{std::move(predicate)} {}

    repeat_on_completed(const repeat_on_completed<Type, SpecificObservable, Predicate>&) = default;

    void operator()(const auto& sub) const
    {
        if (sub.is_subscribed())
        {
            if (m_predicate())
                subscribe_subscriber_for_repeat(sub);
            else
                sub.on_completed();
        }
    }

private:
    void subscribe_subscriber_for_repeat(const constraint::subscriber auto& subscriber) const
    {
        m_shared_observable->subscribe(create_subscriber_with_state<Type>(subscriber.get_subscription().make_child(),
                                                                          subscriber,
                                                                          forwarding_on_next{},
                                                                          forwarding_on_error{},
                                                                          *this));
    }

    std::shared_ptr<SpecificObservable> m_shared_observable;
    [[no_unique_address]] Predicate     m_predicate;
};

struct counted_repeat_predicate
{
    counted_repeat_predicate(size_t count)
        : m_count{std::make_shared<size_t>(count)} {}

    bool operator()() const { return *m_count && (*m_count)--; }
private:
    std::shared_ptr<size_t> m_count{};
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, typename CreatePredicateFn>
struct repeat_on_subscribe
{
    repeat_on_subscribe(TObs&& observable, CreatePredicateFn&& create_predicate)
        : m_shared_observable{std::make_shared<TObs>(std::move(observable))}
        , m_create_predicate{std::move(create_predicate)} {}

    repeat_on_subscribe(const TObs& observable, CreatePredicateFn&& create_predicate)
        : m_shared_observable{ std::make_shared<TObs>(observable) }
        , m_create_predicate{ std::move(create_predicate) } {}

    template<constraint::subscriber_of_type<Type> TSub>
    void operator()(const TSub& subscriber) const
    {
        auto predicate = m_create_predicate();
        repeat_on_completed<Type, std::decay_t<TObs>, decltype(predicate)>{m_shared_observable, std::move(predicate)}(subscriber);
    };

private:
    std::shared_ptr<TObs> m_shared_observable{};
    CreatePredicateFn     m_create_predicate;
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs, typename CreatePredicateFn>
auto create_repeat_on_subscribe(TObs&& observable, CreatePredicateFn&& create_predicate)
{
    return source::create<Type>(repeat_on_subscribe<Type, std::decay_t<TObs>, std::decay_t<CreatePredicateFn>>(std::forward<TObs>(observable),
                                                                                                               std::forward<CreatePredicateFn>(create_predicate)));
}

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto repeat_impl(TObs&& observable, size_t count)
{
    return create_repeat_on_subscribe<Type>(std::forward<TObs>(observable), [count] { return counted_repeat_predicate{count}; });
}

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto repeat_impl(TObs&& observable)
{
    return create_repeat_on_subscribe<Type>(std::forward<TObs>(observable), [] { return [] { return true; }; });
}
} // namespace rpp::details
