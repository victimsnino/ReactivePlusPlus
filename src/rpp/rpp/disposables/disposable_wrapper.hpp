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
    struct weak_tag
    {
    };

    template<std::derived_from<TDisposable> TT = TDisposable>
    explicit disposable_wrapper_impl(weak_tag, std::weak_ptr<TT> disposable)
        requires std::derived_from<TT, interface_disposable>
        : m_disposable{std::move(disposable)}
    {
    }

public:
    disposable_wrapper_impl() = default;

    template<std::derived_from<TDisposable> TT = TDisposable>
    disposable_wrapper_impl(std::shared_ptr<TT>&& disposable)
        requires std::derived_from<TT, interface_disposable>
        : m_disposable{std::static_pointer_cast<TDisposable>(std::move(disposable))}
    {
    }

    template<std::derived_from<TDisposable> TT = TDisposable>
    disposable_wrapper_impl(const std::shared_ptr<TT>& disposable)
        requires std::derived_from<TT, interface_disposable>
        : m_disposable{std::static_pointer_cast<TDisposable>(disposable)}
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

    bool operator==(const disposable_wrapper_impl& other) const
    {
        return raw_pointer() == other.raw_pointer();
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

    void add(disposable_wrapper other) const
        requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        if (const auto locked = get_original())
            locked->add(std::move(other));
        else
            other.dispose();
    }

    void remove(const disposable_wrapper& other) const
        requires std::derived_from<TDisposable, interface_composite_disposable>
    {
        if (const auto locked = get_original())
            locked->remove(other);
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
        if (const auto ptr_ptr = std::get_if<std::shared_ptr<TDisposable>>(&m_disposable))
            return ptr_ptr->use_count() != 0;

        if (const auto ptr_ptr = std::get_if<std::weak_ptr<TDisposable>>(&m_disposable))
            return ptr_ptr->use_count() != 0;

        return false;
    }
private:
    const TDisposable* raw_pointer() const {
        if (const auto ptr_ptr = std::get_if<std::shared_ptr<TDisposable>>(&m_disposable))
            return ptr_ptr->get();

        if (const auto ptr_ptr = std::get_if<std::weak_ptr<TDisposable>>(&m_disposable))
            if (const auto shared = ptr_ptr->lock())
                return shared.get();

        return nullptr;
    }

private:
    std::variant<std::monostate, std::shared_ptr<TDisposable>, std::weak_ptr<TDisposable>> m_disposable;
};
}
