//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/disposables/fwd.hpp>
#include <rpp/observers/fwd.hpp>

#include <rpp/observers/observer.hpp>

#include <memory>
#include <utility>

namespace rpp::details::observers
{
    template<typename Type>
    struct observer_vtable
    {
        void (*on_next_lvalue)(const observer_vtable*, const Type&){};
        void (*on_next_rvalue)(const observer_vtable*, Type&&){};
        void (*on_error)(const observer_vtable*, const std::exception_ptr&){};
        void (*on_completed)(const observer_vtable*){};

        void (*set_upstream)(observer_vtable*, const disposable_wrapper&){};
        bool (*is_disposed)(const observer_vtable*){};
    };

    template<rpp::constraint::observer TObs>
    class type_erased_observer : public observer_vtable<rpp::utils::extract_observer_type_t<TObs>>
    {
        using Type = rpp::utils::extract_observer_type_t<TObs>;
        using Base = observer_vtable<Type>;

        static constexpr const TObs& cast(const Base* ptr)
        {
            return static_cast<const type_erased_observer*>(ptr)->m_observer;
        }

        static constexpr TObs& cast(Base* ptr)
        {
            return static_cast<type_erased_observer*>(ptr)->m_observer;
        }

    public:
        type_erased_observer(TObs&& observer)
            : Base{
                  .on_next_lvalue = +[](const Base* b, const Type& v) { cast(b).on_next(v); },
                  .on_next_rvalue = +[](const Base* b, Type&& v) { cast(b).on_next(std::move(v)); },
                  .on_error       = +[](const Base* b, const std::exception_ptr& err) { cast(b).on_error(err); },
                  .on_completed   = +[](const Base* b) { cast(b).on_completed(); },
                  .set_upstream   = +[](Base* b, const rpp::disposable_wrapper& d) { cast(b).set_upstream(d); },
                  .is_disposed    = +[](const Base* b) { return cast(b).is_disposed(); }
              }
            , m_observer{std::move(observer)}
        {
        }

    private:
        TObs m_observer;
    };

    template<rpp::constraint::decayed_type Type>
    class dynamic_strategy final
    {
    public:
        template<rpp::constraint::observer_strategy<Type> Strategy>
            requires (!rpp::constraint::decayed_same_as<Strategy, dynamic_strategy<Type>>)
        explicit dynamic_strategy(observer<Type, Strategy>&& obs)
            : m_observer{std::make_shared<type_erased_observer<observer<Type, Strategy>>>(std::move(obs))}
        {
        }

        void set_upstream(const disposable_wrapper& d) noexcept { m_observer->set_upstream(m_observer.get(), d); }

        bool is_disposed() const noexcept { return m_observer->is_disposed(m_observer.get()); }

        void on_next(const Type& v) const noexcept { m_observer->on_next_lvalue(m_observer.get(), v); }

        void on_next(Type&& v) const noexcept { m_observer->on_next_rvalue(m_observer.get(), std::move(v)); }

        void on_error(const std::exception_ptr& err) const noexcept { m_observer->on_error(m_observer.get(), err); }

        void on_completed() const noexcept { m_observer->on_completed(m_observer.get()); }
    private:
        std::shared_ptr<observer_vtable<Type>> m_observer;
    };
} // namespace rpp::details::observers


namespace rpp
{
    /**
     * @brief Type-erased version of the `rpp::observer`. Any observer can be converted to dynamic_observer via `rpp::observer::as_dynamic` member function.
     * @details To provide type-erasure it uses `std::shared_ptr`. As a result it has worse performance, but it is **ONLY** way to copy observer.
     *
     * @tparam Type of value this observer can handle
     *
     * @ingroup observers
     */
    template<constraint::decayed_type Type>
    class dynamic_observer final : public observer<Type, details::observers::dynamic_strategy<Type>>
    {
        using base = observer<Type, details::observers::dynamic_strategy<Type>>;

    public:
        using base::base;

        dynamic_observer(base&& b)
            : base{std::move(b)}
        {
        }

        dynamic_observer(const base& b)
            : base{b}
        {
        }
    };
} // namespace rpp
