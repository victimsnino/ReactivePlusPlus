#pragma once

#include <rpp/observables/constraints.hpp>
#include <rpp/operators/fwd/repeat.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/sources/create.hpp>

IMPLEMENTATION_FILE(repeat_tag);

namespace rpp::operators
{
template<typename ...Args>
auto repeat(size_t count) requires details::is_header_included<details::repeat_tag, Args...>
{
    return [count]<constraint::observable TObservable>(TObservable && observable)
    {
        return std::forward<TObservable>(observable).repeat(count);
    };
}

template<typename ...Args>
auto repeat() requires details::is_header_included<details::repeat_tag, Args...>
{
    return[]<constraint::observable TObservable>(TObservable&& observable)
    {
        return std::forward<TObservable>(observable).repeat();
    };
}
} // namespace rpp::operators

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
        if (sub.is_subscribed() 
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

template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::decayed_same_as<SpecificObservable> TThis>
auto member_overload<Type, SpecificObservable, repeat_tag>::repeat_impl(TThis&& observable, size_t count)
{
    auto shared_observable = std::make_shared<SpecificObservable>(std::forward<TThis>(observable));
    return rpp::source::create<Type>([shared_observable, count](const constraint::subscriber_of_type<Type> auto& subscriber)
    {
        auto predicate = [shared_count = std::make_shared<size_t>(count)]()
        {
            return *shared_count && (*shared_count)--;
        };
        repeat_on_completed<Type, SpecificObservable, decltype(predicate)>{shared_observable, std::move(predicate)}(subscriber);
    });
}

template<constraint::decayed_type Type, typename SpecificObservable>
template<constraint::decayed_same_as<SpecificObservable> TThis>
auto member_overload<Type, SpecificObservable, repeat_tag>::repeat_impl(TThis&& observable)
{
    auto shared_observable = std::make_shared<SpecificObservable>(std::forward<TThis>(observable));
    return rpp::source::create<Type>([shared_observable](const constraint::subscriber_of_type<Type> auto& subscriber)
    {
        auto predicate = []() { return true; };
        repeat_on_completed<Type, SpecificObservable, decltype(predicate)>{shared_observable, std::move(predicate)}(subscriber);
    });
}
} // namespace rpp::details
