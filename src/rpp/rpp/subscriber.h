// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <rpp/observer.h>
#include <rpp/subscription.h>
#include <rpp/utils/type_traits.h>

#include <utility>
#include <variant>

namespace rpp
{
template<typename Type>
class subscriber_base : public interface_observer<Type>
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type should be decayed to match with decayed observable types");
public:
    subscriber_base(const subscription& subscription)
        : m_subscription{subscription} {}

    subscriber_base(subscription&& subscription = {})
        : m_subscription{std::move(subscription)} {}

    subscriber_base(const subscriber_base&)     = default;
    subscriber_base(subscriber_base&&) noexcept = default;

    void on_next(const Type& val) const final
    {
        if (!m_subscription.is_subscribed())
            return;

        on_next_impl(val);
    }

    void on_next(Type&& val) const final
    {
        if (!m_subscription.is_subscribed())
            return;

        on_next_impl(std::move(val));
    }

    void on_error(const std::exception_ptr& err) const final
    {
        if (!m_subscription.is_subscribed())
            return;

        subscription_guard guard{m_subscription};
        on_error_impl(err);
    }

    void on_completed() const final
    {
        if (!m_subscription.is_subscribed())
            return;

        subscription_guard guard{m_subscription};
        on_completed_impl();
    }

    const subscription& get_subscription() const
    {
        return m_subscription;
    }

    [[nodiscard]] bool is_subscribed() const
    {
        return m_subscription.is_subscribed();
    }

    void unsubscribe() const
    {
        m_subscription.unsubscribe();
    }

protected:
    virtual void on_next_impl(const Type& v) const = 0;
    virtual void on_next_impl(Type&& v) const = 0;
    virtual void on_error_impl(const std::exception_ptr& err) const = 0;
    virtual void on_completed_impl() const = 0;

private:
    subscription m_subscription;
};

template<typename Type, typename Observer>
class specific_subscriber : public subscriber_base<Type>
{
    static_assert(std::is_same_v<std::decay_t<Observer>, Observer>, "Observer should be decayed");
public:
    //********************* Construct by observer *********************//
    specific_subscriber(const Observer& observer)
        : specific_subscriber{subscription{}, observer} { }

    specific_subscriber(Observer&& observer)
        : specific_subscriber{subscription{}, std::move(observer)} { }

    specific_subscriber(const subscription& sub, const Observer& observer)
        : subscriber_base{sub}
        , m_observer{observer} { }

    specific_subscriber(const subscription& sub, Observer&& observer)
        : subscriber_base{sub}
        , m_observer{std::move(observer)} { }

    //********************* Construct by actions *********************//
    template<typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    specific_subscriber(Types&&...vals)
        : specific_subscriber{subscription{}, std::forward<Types>(vals)...} {}

    template<typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    specific_subscriber(const subscription& sub, Types&&...vals)
        : subscriber_base{sub}
        , m_observer{std::forward<Types>(vals)...} {}

    // ************* Copy/Move ************************* //
    specific_subscriber(const specific_subscriber&)     = default;
    specific_subscriber(specific_subscriber&&) noexcept = default;

    const Observer& get_observer() const
    {
        return m_observer;
    }
protected:
    void on_next_impl(const Type& val) const final
    {
        m_observer.on_next(val);
    }

    void on_next_impl(Type&& val) const final
    {
        m_observer.on_next(std::move(val));
    }

    void on_error_impl(const std::exception_ptr& err) const final
    {
        m_observer.on_error(err);
    }

    void on_completed_impl() const final
    {
        m_observer.on_completed();
    }

private:
    Observer m_observer;
};

template<typename T>
specific_subscriber(dynamic_observer<T> observer) -> specific_subscriber<T, dynamic_observer<T>>;

template<typename T, typename ...Args>
specific_subscriber(specific_observer<T, Args...> observer) -> specific_subscriber<T, specific_observer<T, Args...>>;

template<typename TSub, typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext> && std::is_same_v<TSub, subscription>>>
specific_subscriber(TSub, OnNext, Args ...)->specific_subscriber<std::decay_t<utils::function_argument_t<OnNext>>, specific_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext, Args...>>;

template<typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext>>>
specific_subscriber(OnNext, Args...) -> specific_subscriber<std::decay_t<utils::function_argument_t<OnNext>>, specific_observer<std::decay_t<utils::function_argument_t<OnNext>>, OnNext, Args...>>;

template<typename T>
class dynamic_subscriber : public specific_subscriber<T, dynamic_observer<T>>
{
public:
    using specific_subscriber<T, dynamic_observer<T>>::specific_subscriber;

    template<typename Obs>
    dynamic_subscriber(const specific_subscriber<T, Obs>& subscriber)
        : specific_subscriber{subscriber.get_subscription(), subscriber.get_observer()} {}
};

template<typename T>
dynamic_subscriber(dynamic_observer<T> observer) -> dynamic_subscriber<T>;

template<typename T, typename ...Args>
dynamic_subscriber(specific_observer<T, Args...> observer) -> dynamic_subscriber<T>;

template<typename T, typename Obs>
dynamic_subscriber(specific_subscriber<T, Obs>) -> dynamic_subscriber<T>;

template<typename TSub, typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext> && std::is_same_v<TSub, subscription>>>
dynamic_subscriber(TSub, OnNext, Args ...)->dynamic_subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;

template<typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext>>>
dynamic_subscriber(OnNext, Args...) -> dynamic_subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;
} // namespace rpp
