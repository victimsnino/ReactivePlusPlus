//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/fwd.hpp>
#include <rpp/disposables/fwd.hpp>

#include <memory>
#include <utility>

namespace rpp::details::observer
{
template<typename T, typename Strategy>
void forwarding_on_next_lvalue(const void* const ptr, const T& v) { static_cast<const Strategy*>(ptr)->on_next(v); }

template<typename T, typename Strategy>
void forwarding_on_next_rvalue(const void* const ptr, T&& v) { static_cast<const Strategy*>(ptr)->on_next(std::forward<T>(v)); }

template<typename Strategy>
void forwarding_on_error(const void* const ptr, const std::exception_ptr& err) { static_cast<const Strategy*>(ptr)->on_error(err); }

template<typename Strategy>
void forwarding_on_completed(const void* const ptr) { static_cast<const Strategy*>(ptr)->on_completed(); }

template<typename Strategy>
void forwarding_set_upstream(void* const ptr, const disposable_wrapper& d) { static_cast<Strategy*>(ptr)->set_upstream(d); }

template<typename Strategy>
bool forwarding_is_disposed(const void* const ptr) { return static_cast<const Strategy*>(ptr)->is_disposed(); }

template<constraint::decayed_type Type>
class dynamic_strategy final
{
public:
    template<constraint::observer_strategy<Type> Strategy>
        requires (std::is_rvalue_reference_v<Strategy&&> && !constraint::decayed_same_as<Strategy, dynamic_strategy<Type>>)
    explicit dynamic_strategy(Strategy&& strategy) // NOLINT(bugprone-forwarding-reference-overload)
        : m_forwarder{std::make_shared<Strategy>(std::forward<Strategy>(strategy))}
        , m_vtable{vtable::template create<Strategy>()} {}

    dynamic_strategy(const dynamic_strategy&) = default;
    dynamic_strategy(dynamic_strategy&&) noexcept = default;

    void set_upstream(const disposable_wrapper& d) { m_vtable->set_upstream(m_forwarder.get(), d); }
    bool is_disposed() const                       { return m_vtable->is_disposed(m_forwarder.get()); }

    void on_next(const Type& v) const                     { m_vtable->on_next_lvalue(m_forwarder.get(), v);            }
    void on_next(Type&& v) const                          { m_vtable->on_next_rvalue(m_forwarder.get(), std::move(v)); }
    void on_error(const std::exception_ptr& err) const    { m_vtable->on_error(m_forwarder.get(), err);                }
    void on_completed() const                             { m_vtable->on_completed(m_forwarder.get());                 }

private:
    struct vtable
    {
        void (*on_next_lvalue)(const void*, const Type&){};
        void (*on_next_rvalue)(const void*, Type&&){};
        void (*on_error)(const void*, const std::exception_ptr&){};
        void (*on_completed)(const void*){};

        void (*set_upstream)(void*, const disposable_wrapper&){};
        bool (*is_disposed)(const void*){};

        template<constraint::observer_strategy<Type> Strategy>
        static const vtable* create() noexcept
        {
            static vtable s_res{
                .on_next_lvalue = forwarding_on_next_lvalue<Type, Strategy>,
                .on_next_rvalue = forwarding_on_next_rvalue<Type, Strategy>,
                .on_error = forwarding_on_error<Strategy>,
                .on_completed = forwarding_on_completed<Strategy>,
                .set_upstream = forwarding_set_upstream<Strategy>,
                .is_disposed = forwarding_is_disposed<Strategy>,
            };
            return &s_res;
        }
    };

private:
    std::shared_ptr<void> m_forwarder;
    const vtable*         m_vtable;
};
} // namespace rpp::details::observer
