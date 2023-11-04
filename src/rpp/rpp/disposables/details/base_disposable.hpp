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

#include <atomic>

namespace rpp::details
{
template<typename BaseInterface>
class base_disposable_impl : public BaseInterface
{
public:
    base_disposable_impl()                                = default;
    base_disposable_impl(const base_disposable_impl&)     = delete;
    base_disposable_impl(base_disposable_impl&&) noexcept = delete;

    bool is_disposed() const noexcept final
    {
        // just need atomicity, not guarding anything
        return m_disposed.load(std::memory_order::relaxed);
    }

    void dispose() noexcept final
    {
        // just need atomicity, not guarding anything
        if (m_disposed.exchange(true, std::memory_order::relaxed) == false)
            dispose_impl();
    }

protected:
    virtual void dispose_impl() noexcept = 0;

private:
    std::atomic_bool m_disposed{};
};

using base_disposable           = base_disposable_impl<interface_disposable>;
using base_composite_disposable = base_disposable_impl<interface_composite_disposable>;
}