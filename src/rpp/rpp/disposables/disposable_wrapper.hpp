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
#include <rpp/disposables/base_disposable.hpp>

namespace rpp
{
class disposable_wrapper
{
public:
    explicit disposable_wrapper(std::shared_ptr<base_disposable> disposable = {}) : m_disposable{std::move(disposable)} {}

    bool is_disposed() const { return !m_disposable || m_disposable->is_disposed(); }

    void dispose() const
    {
        if (m_disposable)
            m_disposable->dispose();
    }

    void add(std::shared_ptr<base_disposable> other) const
    {
        if (m_disposable)
            m_disposable->add(std::move(other));
        else if (other)
            other->dispose();
    }
    const std::shared_ptr<base_disposable>& get_original() const { return m_disposable; }

private:
    std::shared_ptr<base_disposable> m_disposable;
};
}