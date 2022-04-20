//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subscribers/dynamic_subscriber.h>
#include <rpp/utils/constraints.h>
#include <rpp/utils/utilities.h>

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace rpp::subjects::details::states
{
template<constraint::decayed_type T>
struct state_interface : public std::enable_shared_from_this<state_interface<T>>
{
    state_interface() = default;

    virtual                                        ~state_interface() = default;
    virtual std::shared_ptr<const state_interface<T>> on_subscribe(const dynamic_subscriber<T>& sub) const = 0;
    virtual std::shared_ptr<const state_interface<T>> on_subscriber_unsubscribed() const { return this->shared_from_this(); }
    virtual bool is_terminated() const { return true; }

    virtual void on_next(const T&) const {}
    virtual void on_error(const std::exception_ptr&) const {}
    virtual void on_unsubscribe() const {  }
    virtual void on_completed() const {}
};

template<constraint::decayed_type T>
class error final : public state_interface<T>
{
public:
    error(const std::exception_ptr& err)
        : m_err{err} {}

    std::shared_ptr<const state_interface<T>> on_subscribe(const dynamic_subscriber<T>& sub) const override
    {
        sub.on_error(m_err);
        return this->shared_from_this();
    }

private:
    const std::exception_ptr m_err;
};

template<constraint::decayed_type T>
struct completed final : public state_interface<T>
{
    std::shared_ptr<const state_interface<T>> on_subscribe(const dynamic_subscriber<T>& sub) const override
    {
        sub.on_completed();
        return this->shared_from_this();
    }

    static auto make()
    {
        static std::shared_ptr<completed> result = std::make_shared<completed>();
        return result;
    }
};

template<constraint::decayed_type T>
struct unsubscribed final : public state_interface<T>
{
    std::shared_ptr<const state_interface<T>> on_subscribe(const dynamic_subscriber<T>& sub) const override
    {
        sub.unsubscribe();
        return this->shared_from_this();
    }

    static auto make()
    {
        static std::shared_ptr<unsubscribed> result = std::make_shared<unsubscribed>();
        return result;
    }
};

template<constraint::decayed_type T>
class active final : public state_interface<T>
{
public:
    active(std::vector<dynamic_subscriber<T>>&& subs = {})
        : m_subs{std::move(subs)} {}

    bool is_terminated() const override { return false; }

    std::shared_ptr<const state_interface<T>> on_subscribe(const dynamic_subscriber<T>& sub) const override
    {
        auto subs = make_copy_of_subscribed_subs(m_subs.size() + 1);
        subs.push_back(sub);
        return std::make_shared<active>(std::move(subs));
    }

    std::shared_ptr<const state_interface<T>> on_subscriber_unsubscribed() const override
    {
        if (m_subs.empty())
            return this->shared_from_this();

        return std::make_shared<active>(make_copy_of_subscribed_subs(m_subs.size() - 1));
    }

    void on_next(const T& v) const override
    {
        std::ranges::for_each(m_subs, [&](const dynamic_subscriber<T>& sub) {sub.on_next(v); });
    }

    void on_error(const std::exception_ptr& err) const override
    {
        std::ranges::for_each(m_subs, [&](const dynamic_subscriber<T>& sub) {sub.on_error(err); });
    }

    void on_completed() const override
    {
        std::ranges::for_each(m_subs, std::mem_fn(&dynamic_subscriber<T>::on_completed));
    }

    void on_unsubscribe() const override
    {
        std::ranges::for_each(m_subs, std::mem_fn(&dynamic_subscriber<T>::unsubscribe));
    }

private:
    std::vector<dynamic_subscriber<T>> make_copy_of_subscribed_subs(size_t expected_size) const
    {
        std::vector<dynamic_subscriber<T>> subs{};
        subs.reserve(expected_size);
        std::ranges::copy_if(m_subs, std::back_inserter(subs), std::mem_fn(&dynamic_subscriber<T>::is_subscribed));
        return subs;
    }

private:
    const std::vector<dynamic_subscriber<T>> m_subs{};
};
} // namespace rpp::subjects::details::states

namespace rpp::subjects::details
{
template<constraint::decayed_type T>
class subject_state : public std::enable_shared_from_this<subject_state<T>>
{
public:
    using subscriber = dynamic_subscriber<T>;

    subject_state() = default;

    void on_subscribe(const subscriber& subscriber)
    {
        while (true)
        {
            auto current_state = std::atomic_load(&m_state);
            auto new_state     = current_state->on_subscribe(subscriber);
            if (current_state == new_state)
                return;

            if (!std::atomic_compare_exchange_strong(&m_state, &current_state, new_state))
                continue;

            auto weak = this->weak_from_this();
            subscriber.get_subscription().add([weak]
            {
                while (auto shared = weak.lock())
                {
                    if (shared->try_to_update_state(&states::state_interface<T>::on_subscriber_unsubscribed))
                        return;
                }
            });
            return;
        }
    }

    void on_next(const T& v)
    {
        std::atomic_load(&m_state)->on_next(v);
    }

    void on_error(const std::exception_ptr& err)
    {
        update_state_if_not_terminated(std::make_shared<states::error<T>>(err), [&](const auto& state) { state->on_error(err); });
    }

    void on_completed()
    {
        update_state_if_not_terminated(states::completed<T>::make(), &states::state_interface<T>::on_completed);
    }

    void on_unsubscribe()
    {
        update_state_if_not_terminated(states::unsubscribed<T>::make(), &states::state_interface<T>::on_unsubscribe);
    }

private:
    using state_interface_transition_fn = std::shared_ptr<const states::state_interface<T>>(states::state_interface<T>::*)() const;
    bool try_to_update_state(state_interface_transition_fn function)
    {
        auto current_state = std::atomic_load(&m_state);
        auto new_state = std::invoke(function, current_state);

        return try_to_update_state(current_state, new_state);
    }

    bool try_to_update_state(std::shared_ptr<const states::state_interface<T>> old, std::shared_ptr<const states::state_interface<T>> new_state)
    {
        return old == new_state || std::atomic_compare_exchange_strong(&m_state, &old, new_state);
    }

    void update_state_if_not_terminated(std::shared_ptr<const states::state_interface<T>> new_state, const auto& state_callback)
    {
        while (true)
        {
            auto current_state = std::atomic_load(&m_state);
            if (current_state->is_terminated())
                return;

            if (!try_to_update_state(current_state, new_state))
                continue;

            std::invoke(state_callback, current_state);
            return;
        }
    }

private:
    utils::atomic_shared_ptr<const states::state_interface<T>> m_state = std::make_shared<states::active<T>>();
};
} // namespace rpp::subjects::details
