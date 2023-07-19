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
#include <rpp/disposables/details/base_disposable.hpp>

namespace rpp
{
template<std::invocable Fn>
class callback_disposable final : public details::base_disposable
{
public:
    callback_disposable(Fn&& fn) : m_fn{std::move(fn)} {}
    callback_disposable(const Fn& fn) : m_fn{fn} {}

private:
    void dispose_impl() override { std::move(m_fn)(); }

private:
    Fn m_fn;
};

template<std::invocable Fn>
auto make_callback_disposable(Fn&& invocable)
{
    return std::make_shared<rpp::callback_disposable<std::decay_t<Fn>>>(std::forward<Fn>(invocable));
}
} // namespace rpp