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
namespace details
{
    enum class SubscriberStrategy
    {
        Local, // delay using heap as long as possible. Useful when you use chain with no actual requisitions to
        Shared // make subscription as shared to provide ability to share it with others
    };
}

template<typename Type, details::SubscriberStrategy strategy>
class subscriber_base
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type should be decayed to match with decayed observable types");

    template<typename TType>
    using enable_if_same_type_t = std::enable_if_t<std::is_same_v<std::decay_t<TType>, Type>>;

    template<typename ...Types>
    static constexpr bool is_observer_constructible_v = (rpp::utils::is_callable_v<Types> && ...) && (std::is_constructible_v<observer<Type>, Types...> ||
        std::is_constructible_v<observer<Type&>, Types...> ||
        std::is_constructible_v<observer<const Type&>, Types...> ||
        std::is_constructible_v<observer<Type&&>, Types...>);
public:
    template<typename TType, typename = enable_if_same_type_t<TType>>
    subscriber_base(const observer<TType>& observer)
        : m_observer{observer}
        , m_subscription{make_subscription()} { }

    template<typename TType, typename = enable_if_same_type_t<TType>>
    subscriber_base(observer<TType>&& observer)
        : m_observer{std::move(observer)}
        , m_subscription{make_subscription()} { }

    template<typename ...Types,
             typename Enabled = std::enable_if_t<is_observer_constructible_v<Types...>>>
    subscriber_base(Types&&...vals)
        : subscriber_base{make_subscription(), std::forward<Types>(vals)...} {}

    template<details::SubscriberStrategy strat>
    subscriber_base(const subscriber_base<Type, strat>& sub)
        : m_observer{ sub.m_observer }
        , m_subscription{ sub.get_subscription().as_shared() } {}

    template<details::SubscriberStrategy strat>
    subscriber_base(subscriber_base<Type, strat>&& sub)
        : m_observer{ std::move(sub.m_observer) }
        , m_subscription{ sub.get_subscription() } {}

    template<typename ...Types, typename = std::enable_if_t<is_observer_constructible_v<Types...>>>
    subscriber_base(const subscription& sub, Types&&...vals)
        : m_observer{observer{std::forward<Types>(vals)...}}
        , m_subscription{strategy == details::SubscriberStrategy::Local ? sub : sub.as_shared()} {}

    virtual ~subscriber_base() = default;

    template<typename U, typename = enable_if_same_type_t<U>>
    void on_next(U&& val) const
    {
        if (!m_subscription.is_subscribed())
            return;

        std::visit([&](auto& obs) { obs.on_next(std::forward<U>(val)); }, m_observer);
    }

    void on_error(std::exception_ptr err) const
    {
        if (!m_subscription.is_subscribed())
            return;

        subscription_guard   guard{m_subscription};
        std::visit([&](auto& obs) { obs.on_error(err); }, m_observer);
    }

    void on_completed() const
    {
        if (!m_subscription.is_subscribed())
            return;

        subscription_guard   guard{m_subscription};
        std::visit([&](auto& obs) { obs.on_completed(); }, m_observer);
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
    subscription make_subscription() const
    {
        if (strategy == details::SubscriberStrategy::Local)
            return details::subscription_state{};
        return std::make_shared<details::subscription_state>();
    }

private:
    std::variant<observer<Type>,
                 observer<Type&>,
                 observer<const Type&>,
                 observer<Type&&>> m_observer;

    subscription m_subscription;

    friend class subscriber_base<Type, details::SubscriberStrategy::Local>;
    friend class subscriber_base<Type, details::SubscriberStrategy::Shared>;
};

template<typename Type>
class local_subscriber final: public subscriber_base<Type, details::SubscriberStrategy::Local>
{
public:
    using subscriber_base<Type, details::SubscriberStrategy::Local>::subscriber_base;

    local_subscriber(const local_subscriber<Type>& o) = delete;

};

template<typename Type>
class subscriber final : public subscriber_base<Type, details::SubscriberStrategy::Shared>
{
public:
    using subscriber_base<Type, details::SubscriberStrategy::Shared>::subscriber_base;

    subscriber(const subscriber<Type>& o) : subscriber_base{o} {}
    subscriber(subscriber<Type>&& o) noexcept : subscriber_base{std::move(o)} {}
};

template<typename T>
subscriber(observer<T> observer) -> subscriber<std::decay_t<T>>;

template<typename TSub, typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext> && 
                                                                                      (rpp::utils::is_subscriber_v<TSub> || std::is_same_v<TSub, subscription>)>>
subscriber(TSub, OnNext, Args ...)->subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;

template<typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext>>>
subscriber(OnNext, Args...) -> subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;


template<typename T>
local_subscriber(observer<T> observer) -> local_subscriber<std::decay_t<T>>;

template<typename TSub, typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext> && 
                                                                                      (rpp::utils::is_subscriber_v<TSub> || std::is_same_v<TSub, subscription>)>>
local_subscriber(TSub, OnNext, Args ...)->local_subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;

template<typename OnNext, typename ...Args, typename = std::enable_if_t<utils::is_callable_v<OnNext>>>
local_subscriber(OnNext, Args...) -> local_subscriber<std::decay_t<utils::function_argument_t<OnNext>>>;
} // namespace rpp
