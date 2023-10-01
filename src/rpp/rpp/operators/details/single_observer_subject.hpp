//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subjects/fwd.hpp>

#include <rpp/observers/observer.hpp>
#include <rpp/subjects/details/base_subject.hpp>
#include <rpp/subjects/details/subject_state.hpp>
#include <rpp/disposables/refcount_disposable.hpp>

#include <rpp/utils/exceptions.hpp>

#include <memory>

namespace rpp::operators::details
{
template<rpp::constraint::decayed_type Type>
class single_observer_strategy
{
    class state_t final : public composite_disposable
    {
    public:
        state_t() = default;
        ~state_t() noexcept
        {
            dispose_impl();
        }

        void emplace(rpp::dynamic_observer<Type>&& observer)
        {
            {
                std::lock_guard lock{m_mutex};
                if (m_state == EState::Empty)
                {
                    std::construct_at(get(), std::move(observer));
                    m_state = EState::Observer;
                    return;
                }
            }
            observer.on_error(std::make_exception_ptr(rpp::utils::more_than_one_observer{"single_observer_strategy just has non-empty observer, but expected only one observer at the same time"}));
        }

        rpp::dynamic_observer<Type> get_observer() 
        {
            std::lock_guard lock{m_mutex};
            return m_state == EState::Observer ? *get() : rpp::dynamic_observer<Type>{};
        }
        
    private:
        void dispose_impl() noexcept override
        {
            std::lock_guard lock{m_mutex};
            if (m_state == EState::Observer)
                std::destroy_at(get());
            m_state = EState::Disposed;
        }

        rpp::dynamic_observer<Type>* get()
        {
            return reinterpret_cast<rpp::dynamic_observer<Type>*>(m_data);
        }

    private:
        enum class EState : uint8_t
        {
            Empty,
            Observer,
            Disposed
        };

        alignas(rpp::dynamic_observer<Type>) std::byte m_data[sizeof(rpp::dynamic_observer<Type>)]{};
        std::mutex m_mutex{};
        EState     m_state{};
    };

    struct observer_strategy
    {
        std::shared_ptr<state_t> state{};

        static void set_upstream(const disposable_wrapper&) {}
        static bool is_disposed() {return false;}

        void on_next(const Type& v) const { state->get_observer().on_next(v); }

        void on_error(const std::exception_ptr& err) const { state->get_observer().on_error(err); }

        void on_completed() const { state->get_observer().on_completed(); }
    };

public:
    explicit single_observer_strategy(std::shared_ptr<rpp::refcount_disposable> refcount)
        : m_refcount{std::move(refcount)}
    {
        m_refcount->add(rpp::disposable_wrapper::from_weak(m_state));
    }

    auto get_observer() const
    {
        return rpp::observer<Type, rpp::details::with_disposable<observer_strategy>>{composite_disposable_wrapper{m_state}, observer_strategy{m_state}};
    }

    template<rpp::constraint::observer_of_type<Type> TObs>
    void on_subscribe(TObs&& observer) const
    {
        observer.set_upstream(m_refcount->add_ref());
        observer.set_upstream(rpp::disposable_wrapper::from_weak(m_state));
        m_state->emplace(std::forward<TObs>(observer).as_dynamic());
    }

    rpp::disposable_wrapper get_disposable() const
    {
        return rpp::disposable_wrapper{m_state};
    }

private:
    std::shared_ptr<state_t>                  m_state = std::make_shared<state_t>();
    std::shared_ptr<rpp::refcount_disposable> m_refcount{};
};

template<rpp::constraint::decayed_type Type>
using single_observer_subject = subjects::details::base_subject<Type, single_observer_strategy<Type>>;
} 