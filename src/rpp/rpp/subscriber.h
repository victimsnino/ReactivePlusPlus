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
template<typename Type, typename Observer = rpp::dynamic_observer<Type>>
class subscriber final : public interface_observer<Type>
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type should be decayed to match with decayed observable types");

public:
    //********************* Construct by observer *********************//
    subscriber(const dynamic_observer<Type>& observer)
        : m_observer{observer} { }

    subscriber(dynamic_observer<Type>&& observer)
        : m_observer{std::move(observer)} { }

    template<typename ...Fns>
    subscriber(const specific_observer<Type, Fns...>& observer)
        : m_observer{observer} { }

    template<typename ...Fns>
    subscriber(specific_observer<Type, Fns...>&& observer)
        : m_observer{std::move(observer)} { }

    //********************* Construct by actions *********************//
    template<typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    subscriber(Types&&...vals)
        : subscriber{subscription{}, std::forward<Types>(vals)...} {}

    template<typename TType,
             typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    subscriber(const subscriber<TType>& sub, Types&&...vals)
        :subscriber{sub.get_subscription(), std::forward<Types>(vals)...} {}

    template<typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    subscriber(const subscription& sub, Types&&...vals)
        : m_observer{std::forward<Types>(vals)...}
        , m_subscription{sub} {}

    void on_next(const Type& val) const override
    {
        if (!m_subscription.is_subscribed())
            return;

        m_observer.on_next(val);
    }

    void on_next(Type&& val) const override
    {
        if (!m_subscription.is_subscribed())
            return;

        m_observer.on_next(std::move(val));
    }

    void on_error(const std::exception_ptr& err) const override
    {
        if (!m_subscription.is_subscribed())
            return;

        subscription_guard   guard{m_subscription};
        m_observer.on_error(err);
    }

    void on_completed() const override
    {
        if (!m_subscription.is_subscribed())
            return;

        subscription_guard   guard{m_subscription};
        m_observer.on_completed();
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

private:
    Observer     m_observer;
    subscription m_subscription;
};

template<typename T>
subscriber(dynamic_observer<T> observer) -> subscriber<T>;

template<typename T, typename OnNext, typename OnError, typename OnCompleted>
subscriber(specific_observer<T, OnNext, OnError, OnCompleted> observer) -> subscriber<T, specific_observer<T, OnNext, OnError, OnCompleted>>;

template<typename TSub, typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext> && 
                                                                                      (rpp::utils::is_subscriber_v<TSub> || std::is_same_v<TSub, subscription>)>>
subscriber(TSub, OnNext, Args ...)->subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;

template<typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext>>>
subscriber(OnNext, Args...) -> subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;
} // namespace rpp
