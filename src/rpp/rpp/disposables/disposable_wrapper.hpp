//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/defs.hpp>
#include <rpp/disposables/fwd.hpp>
#include <rpp/disposables/interface_disposable.hpp>
#include <rpp/utils/utils.hpp>

#include <memory>
#include <variant>

namespace rpp::details
{
template<rpp::constraint::decayed_type TDisposable>
class enable_wrapper_from_this;

template<rpp::constraint::decayed_type TDisposable>
class auto_dispose_wrapper final
{
public:
    static_assert(std::derived_from<TDisposable, interface_disposable>);

    template<typename... TArgs>
        requires (std::constructible_from<TDisposable, TArgs&&...> && !rpp::constraint::variadic_decayed_same_as<auto_dispose_wrapper, TArgs...>)
    explicit auto_dispose_wrapper(TArgs&&... args)
        : m_data{std::forward<TArgs>(args)...}
    {
    }

    auto_dispose_wrapper(const auto_dispose_wrapper&) = delete;
    auto_dispose_wrapper(auto_dispose_wrapper&&) noexcept = delete;

    ~auto_dispose_wrapper() noexcept
    {
        static_cast<interface_disposable&>(m_data).dispose_impl(rpp::interface_disposable::Mode::Destroying);
    }

    TDisposable* get() { return &m_data; }

private:
    RPP_NO_UNIQUE_ADDRESS TDisposable m_data;
};

class disposable_wrapper_base
{
public:
    bool operator==(const disposable_wrapper_base& other) const
    {
        return get().first == other.get().first;
    }

    bool is_disposed() const noexcept
    {
        if (const auto locked = get().first)
            return locked->is_disposed();
        return true;
    }

    void dispose() const noexcept
    {
        if (const auto locked = get().first)
            locked->dispose();
    }

protected:
    explicit disposable_wrapper_base(std::shared_ptr<interface_disposable>&& disposable)
        : m_disposable{std::move(disposable)}
    {
    }

    explicit disposable_wrapper_base(std::weak_ptr<interface_disposable>&& disposable)
        : m_disposable{std::move(disposable)}
    {
    }

    disposable_wrapper_base() = default;

    std::pair<std::shared_ptr<interface_disposable>, bool> get() const noexcept
    {
        if (const auto ptr_ptr = std::get_if<std::shared_ptr<interface_disposable>>(&m_disposable))
            return {*ptr_ptr, true};

        if (const auto ptr_ptr = std::get_if<std::weak_ptr<interface_disposable>>(&m_disposable))
            return {ptr_ptr->lock(), false};

        return {nullptr, true};
    }

private:
    std::variant<std::monostate, std::shared_ptr<interface_disposable>, std::weak_ptr<interface_disposable>> m_disposable;
};

}

namespace rpp
{
/**
 * @brief Wrapper to keep disposable. Any disposable have to be created right from this wrapper with help of `make` function. 
 * @details Member functions is safe to call even if internal disposable is gone. Also  it provides access to "raw" shared_ptr and it can be nullptr in case of disposable empty/ptr gone.
 * @details Can keep weak_ptr in case of not owning disposable
 *
 * @ingroup disposables
 */
template<rpp::constraint::decayed_type TDisposable>
class disposable_wrapper_impl final : public details::disposable_wrapper_base
{
    using TDefaultMake = std::conditional_t<std::same_as<TDisposable, interface_composite_disposable>, composite_disposable, TDisposable>;
public:
    template<constraint::decayed_type TTarget>
    friend class disposable_wrapper_impl;

    template<rpp::constraint::decayed_type TTarget>
    friend class details::enable_wrapper_from_this;

    bool operator==(const disposable_wrapper_impl&) const = default;

    /**
     * @brief Way to create disposable_wrapper. Passed `TTarget` type can be any type derived from `TDisposable`.
     */
    template<std::derived_from<TDisposable> TTarget = TDefaultMake, typename... TArgs>
        requires (std::constructible_from<TTarget, TArgs&&...>)
    static disposable_wrapper_impl make(TArgs&& ...args)
    {
        const auto ptr = std::make_shared<details::auto_dispose_wrapper<TTarget>>(std::forward<TArgs>(args)...);
        auto base_ptr = std::shared_ptr<TDisposable>{ptr, static_cast<TDisposable*>(ptr->get())};
        if constexpr (rpp::utils::is_base_of_v<TDisposable, rpp::details::enable_wrapper_from_this>)
        {
            base_ptr->set_weak_self(std::weak_ptr<interface_disposable>(base_ptr));
        }
        return disposable_wrapper_impl{std::static_pointer_cast<interface_disposable>(std::move(base_ptr))};
    }

    /**
     * @brief Creates disposable_wrapper which behaves like disposed disposable
     */
    static disposable_wrapper_impl empty()
    {
        return disposable_wrapper_impl{};
    }

    template<rpp::constraint::is_nothrow_invocable Fn>
    disposable_wrapper add(Fn&& invocable) requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        auto d = make_callback_disposable(std::forward<Fn>(invocable));
        add(d);
        return d;
    }

    void add(disposable_wrapper other) const requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        if (const auto locked = lock())
            locked->add(std::move(other));
        else
            other.dispose();
    }

    void remove(const disposable_wrapper& other) const requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        if (const auto locked = lock())
            locked->remove(other);
    }

    void clear() const requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        if (const auto locked = lock())
            locked->clear();
    }

    std::shared_ptr<TDisposable> lock() const noexcept
    {
        return std::static_pointer_cast<TDisposable>(get().first);
    }

    disposable_wrapper_impl as_weak() const
    {
        auto [locked, is_shared] = get();
        if (is_shared)
            return disposable_wrapper_impl{std::weak_ptr<interface_disposable>{locked}};
        return *this;
    }

    template<constraint::decayed_type TTarget>
        requires rpp::constraint::static_pointer_convertible_to<TDisposable, TTarget>
    operator disposable_wrapper_impl<TTarget>() const
    {
        auto [locked, is_shared] = get();
        if (!locked)
            return rpp::disposable_wrapper_impl<TTarget>::empty();
        
        auto res = disposable_wrapper_impl<TTarget>{std::move(locked)};
        if (is_shared)
            return res;

        return res.as_weak();
    }
private:
    using details::disposable_wrapper_base::disposable_wrapper_base;
};
}

namespace rpp::details
{
template<rpp::constraint::decayed_type TStrategy>
class enable_wrapper_from_this
{
public:
    template<rpp::constraint::decayed_type TSource>
    friend class rpp::disposable_wrapper_impl;

protected:
    enable_wrapper_from_this() = default;

    void set_weak_self(std::weak_ptr<interface_disposable> weak)
    {
        m_weak = std::move(weak);
    }

public:
    disposable_wrapper_impl<TStrategy> wrapper_from_this() const
    {
        return disposable_wrapper_impl<TStrategy>(std::static_pointer_cast<interface_disposable>(m_weak.lock()));
    }

private:
    std::weak_ptr<interface_disposable> m_weak{};
};
}