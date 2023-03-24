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

#include <memory>

namespace rpp::details
{
template<typename T, typename Strategy>
void forwarding_on_next_lvalue(const void* ptr, const T& v) { static_cast<const Strategy*>(ptr)->on_next(v); }

template<typename T, typename Strategy>
void forwarding_on_next_rvalue(const void* ptr, T&& v) { static_cast<const Strategy*>(ptr)->on_next(std::move(v)); }

template<typename Strategy>
void forwarding_on_error(const void* ptr, const std::exception_ptr& err) { static_cast<const Strategy*>(ptr)->on_error(err); }

template<typename Strategy>
void forwarding_on_completed(const void* ptr) { static_cast<const Strategy*>(ptr)->on_completed(); };

template<constraint::decayed_type Type>
class dynamic_strategy final
{
public:
    template<constraint::observer_strategy<Type> Strategy>
        requires (!constraint::decayed_same_as<Strategy, dynamic_strategy<Type>>)
    dynamic_strategy(Strategy&& strategy)
        : m_forwarder{std::make_shared<Strategy>(std::move(strategy))}
        , m_vtable{vtable::template create<Strategy>()} {}

    dynamic_strategy(const dynamic_strategy&) = default;
    dynamic_strategy(dynamic_strategy&&) noexcept = default;

    void on_next(const Type& v) const noexcept                  { m_vtable->on_next_lvalue(m_forwarder, v);            }
    void on_next(Type&& v) const noexcept                       { m_vtable->on_next_rvalue(m_forwarder, std::move(v)); }
    void on_error(const std::exception_ptr& err) const noexcept { m_vtable->on_error(m_forwarder, err);                }
    void on_completed() const noexcept                          { m_vtable->on_completed(m_forwarder);                 }

private:
    struct vtable
    {
        void (*on_next_lvalue)(const void*, const Type&){};
        void (*on_next_rvalue)(const void*, Type&&){};
        void (*on_error)(const void*, const std::exception_ptr&){};
        void (*on_completed)(const void*){};

        template<constraint::observer_strategy<Type> Strategy>
        static const vtable* create() noexcept
        {
            static vtable s_res{
                .on_next_lvalue = forwarding_on_next_lvalue<Type, Strategy>,
                .on_next_rvalue = forwarding_on_next_rvalue<Type, Strategy>,
                .on_error = forwarding_on_error<Strategy>,
                .on_completed = forwarding_on_completed<Strategy>
            };
            return &s_res;
        }
    };

private:
    std::shared_ptr<void> m_forwarder;
    const vtable*         m_vtable;
};
} // namespace rpp::details
