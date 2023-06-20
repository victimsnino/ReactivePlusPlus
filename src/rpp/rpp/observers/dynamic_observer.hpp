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
#include <rpp/disposables/fwd.hpp>

#include <memory>
#include <utility>

namespace rpp::details::observers
{
template<constraint::decayed_type Type>
class dynamic_strategy final
{
public:
    template<constraint::observer_strategy<Type> Strategy>
        requires (!constraint::decayed_same_as<Strategy, dynamic_strategy<Type>>)
    explicit dynamic_strategy(observer<Type, Strategy>&& obs)
        : m_forwarder{std::make_shared<observer<Type, Strategy>>(std::move(obs))}
        , m_vtable{observer_vtable<Type>::template create<observer<Type, Strategy>>()} {}

    dynamic_strategy(const dynamic_strategy&) = default;
    dynamic_strategy(dynamic_strategy&&) noexcept = default;

    void set_upstream(const disposable_wrapper& d) { m_vtable->set_upstream(m_forwarder.get(), d); }
    bool is_disposed() const                       { return m_vtable->is_disposed(m_forwarder.get()); }

    void on_next(const Type& v) const noexcept                     { m_vtable->on_next_lvalue(m_forwarder.get(), v);            }
    void on_next(Type&& v) const noexcept                          { m_vtable->on_next_rvalue(m_forwarder.get(), std::move(v)); }
    void on_error(const std::exception_ptr& err) const noexcept    { m_vtable->on_error(m_forwarder.get(), err);                }
    void on_completed() const noexcept                             { m_vtable->on_completed(m_forwarder.get());                 }


private:
    std::shared_ptr<void>        m_forwarder;
    const observer_vtable<Type>* m_vtable;
};
} // namespace rpp::details::observers