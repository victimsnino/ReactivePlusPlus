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

#include <rpp/disposables/fwd.hpp>
#include <rpp/disposables/interface_disposable.hpp>

#include <memory>
#include <variant>

namespace rpp
{
/**
 * @brief Wrapper over disposable_ptr to prevent manual checking over nullptr/is_disposed()
 * @details Can keep weak_ptr in case of not owning disposable
 *
 * @ingroup disposables
 */
template<rpp::constraint::decayed_type TDisposable>
class disposable_wrapper_impl
{
    struct weak_tag{};

    template<std::derived_from<TDisposable> TT = TDisposable>
    explicit disposable_wrapper_impl(weak_tag, std::weak_ptr<TT> disposable)
        requires std::derived_from<TT, interface_disposable>
        : m_disposable{std::move(disposable)}
    {
    }

public:
    template<std::derived_from<TDisposable> TT = TDisposable>
    disposable_wrapper_impl(std::shared_ptr<TT>&& disposable = {}, bool can_be_replaced_on_set_upstream = false)
        requires std::derived_from<TT, interface_disposable>
        : m_disposable{std::static_pointer_cast<TDisposable>(std::move(disposable))}
        , m_can_be_replaced_on_set_upstream{can_be_replaced_on_set_upstream}
    {
    }

    template<std::derived_from<TDisposable> TT = TDisposable>
    disposable_wrapper_impl(const std::shared_ptr<TT>& disposable, bool can_be_replaced_on_set_upstream = false)
        requires std::derived_from<TT, interface_disposable>
        : m_disposable{std::static_pointer_cast<TDisposable>(disposable)}
        , m_can_be_replaced_on_set_upstream{can_be_replaced_on_set_upstream}
    {
    }

    static disposable_wrapper_impl from_shared(std::shared_ptr<TDisposable> disposable)
    {
        return disposable_wrapper_impl{std::move(disposable)};
    }

    static disposable_wrapper_impl from_weak(std::weak_ptr<TDisposable> disposable)
    {
        return disposable_wrapper_impl{weak_tag{}, std::move(disposable)};
    }

    static disposable_wrapper_impl with_can_be_replaced_on_set_upstream(disposable_wrapper_impl other)
    {
        other.m_can_be_replaced_on_set_upstream = true;
        return other;
    }

    bool is_disposed() const noexcept
    {
        if (const auto locked = get_original())
            return locked->is_disposed();
        return true;
    }

    void dispose() const noexcept
    {
        if (const auto locked = get_original())
            locked->dispose();
    }

    void add(disposable_wrapper other) const requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        if (const auto locked = get_original())
            locked->add(std::move(other));
        else
            other.dispose();
    }

    std::shared_ptr<TDisposable> get_original() const noexcept
    {
        if (const auto ptr_ptr = std::get_if<std::shared_ptr<TDisposable>>(&m_disposable))
            return *ptr_ptr;

        if (const auto ptr_ptr = std::get_if<std::weak_ptr<TDisposable>>(&m_disposable))
            return ptr_ptr->lock();

        return nullptr;
    }

    operator disposable_wrapper_impl<interface_disposable>() const
    {
        if (const auto ptr_ptr = std::get_if<std::shared_ptr<TDisposable>>(&m_disposable))
            return disposable_wrapper::from_shared(*ptr_ptr);

        if (const auto ptr_ptr = std::get_if<std::weak_ptr<TDisposable>>(&m_disposable))
            return disposable_wrapper::from_weak(*ptr_ptr);

        return rpp::disposable_wrapper{};
    }

    bool has_underlying() const
    {
        return std::visit([](const auto& ptr) { return ptr.use_count() != 0; }, m_disposable);
    }

    bool can_be_replaced_on_set_upstream() const { return m_can_be_replaced_on_set_upstream; }

private:
    std::variant<std::shared_ptr<TDisposable>, std::weak_ptr<TDisposable>> m_disposable;
    bool m_can_be_replaced_on_set_upstream{};
};
}
