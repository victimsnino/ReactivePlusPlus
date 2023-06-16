//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include "rpp/utils/constraints.hpp"
#include <rpp/observers/fwd.hpp>
#include <rpp/observers/details/observer_vtable.hpp>

namespace rpp::details::observers
{
template<typename T>
struct construct_with{};
template<constraint::decayed_type Type, size_t size, size_t alignment>
class type_erased_strategy final
{
public:
    template<constraint::observer_strategy<Type> Strategy, typename ...Args>
        requires(sizeof(Strategy) == size && alignof(Strategy) == alignment && constraint::is_constructible_from<Strategy, Args&&...>)
    explicit type_erased_strategy(construct_with<Strategy>, Args&& ...args)
        : m_vtable{observer_vtable<Type>::template create<std::decay_t<Strategy>>()}
    {
        std::construct_at(reinterpret_cast<std::decay_t<Strategy>*>(m_data), std::forward<Args>(args)...);
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

    void on_next(const Type& v) const                     { m_vtable->on_next_lvalue(m_data, v);            }
    void on_next(Type&& v) const                          { m_vtable->on_next_rvalue(m_data, std::move(v)); }
    void on_error(const std::exception_ptr& err) const    { m_vtable->on_error(m_data, err);                }
    void on_completed() const                             { m_vtable->on_completed(m_data);                 }


private:
    alignas(alignment) std::byte m_data[size]{};
    const observer_vtable<Type>* m_vtable;
};
}