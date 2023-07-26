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
template<rpp::constraint::decayed_type TDisposable>
class disposable_wrapper_impl
{
    struct weak_tag{};

    explicit disposable_wrapper_impl(weak_tag, std::weak_ptr<TDisposable> disposable)
        requires std::derived_from<TDisposable, interface_disposable>
        : m_disposable{std::move(disposable)}
    {
    }

public:
    disposable_wrapper_impl(std::shared_ptr<TDisposable> disposable = {})
        requires std::derived_from<TDisposable, interface_disposable>
    : m_disposable{std::move(disposable)}
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

    bool is_disposed() const noexcept
    {
        if (const auto locked = get_original())
            return locked->is_disposed();
        return true;
    }

    void dispose() const
    {
        if (const auto locked = get_original())
            locked->dispose();
    }

    void add(disposable_wrapper other) const requires std::same_as<TDisposable, composite_disposable>
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

        return disposable_wrapper{};
    }

private:
    std::variant<std::shared_ptr<TDisposable>, std::weak_ptr<TDisposable>> m_disposable;
};

template<typename TDisposable>
disposable_wrapper_impl(const std::shared_ptr<TDisposable>&) -> disposable_wrapper_impl<TDisposable>;
}
