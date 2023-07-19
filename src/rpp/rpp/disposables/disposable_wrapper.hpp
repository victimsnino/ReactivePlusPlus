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

namespace rpp
{
template<rpp::constraint::decayed_type TDisposable>
class disposable_wrapper_impl
{
public:

    disposable_wrapper_impl(std::shared_ptr<TDisposable> disposable = {})
        requires std::derived_from<TDisposable, interface_disposable>
    : m_disposable{std::move(disposable)}
    {
    }

    bool is_disposed() const noexcept { return !m_disposable || m_disposable->is_disposed(); }

    void dispose() const
    {
        if (m_disposable)
            m_disposable->dispose();
    }

    void add(std::shared_ptr<interface_disposable> other) const requires std::same_as<TDisposable, composite_disposable>
    {
        if (m_disposable)
            m_disposable->add(std::move(other));
        else if (other)
            other->dispose();
    }
    const std::shared_ptr<TDisposable>& get_original() const { return m_disposable; }

    operator disposable_wrapper_impl<interface_disposable>() const {return disposable_wrapper{m_disposable}; }

private:
    std::shared_ptr<TDisposable> m_disposable;
};
}