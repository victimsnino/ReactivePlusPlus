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
#include <rpp/observers/details/observer_vtable.hpp>

namespace rpp::details::observers
{
template<constraint::decayed_type Type, size_t size, size_t alignment>
class type_erased_strategy final
{
public:
    template<constraint::observer_strategy<Type> Strategy>
        requires(sizeof(std::decay_t<Strategy>) == size && alignof(std::decay_t<Strategy>) == alignment && !constraint::decayed_same_as<Strategy, type_erased_strategy<Type, size, alignment>>)
    explicit type_erased_strategy(Strategy&& str) 
        : m_vtable{observer_vtable<Type>::template create<std::decay_t<Strategy>>()}
    {
        std::construct_at(static_cast<std::decay_t<Strategy>*>(str), std::forward<Strategy>(str));
    }

    type_erased_strategy(const type_erased_strategy&) = delete;
    type_erased_strategy(type_erased_strategy&& other) noexcept 
        : m_vtable(other.m_vtable)
    {
        m_vtable->move_to(other.m_data, m_data);
    }

    ~type_erased_strategy() noexcept
    {
        m_vtable->destroy_at(m_data);
    }

    void set_upstream(const disposable_wrapper& d) { m_vtable->set_upstream(m_data, d); }
    bool is_disposed() const                       { return m_vtable->is_disposed(m_data); }

    void on_next(const Type& v) const noexcept                     { m_vtable->on_next_lvalue(m_data, v);            }
    void on_next(Type&& v) const noexcept                          { m_vtable->on_next_rvalue(m_data, std::move(v)); }
    void on_error(const std::exception_ptr& err) const noexcept    { m_vtable->on_error(m_data, err);                }
    void on_completed() const noexcept                             { m_vtable->on_completed(m_data);                 }


private:
    alignas(alignment) std::byte m_data[size]{};
    const observer_vtable<Type>* m_vtable;
};

template<constraint::decayed_type Type, constraint::observer_strategy<Type> Strategy>
using type_erased_strategy_for = type_erased_strategy<Type, sizeof(Strategy), alignof(Strategy)>;
}