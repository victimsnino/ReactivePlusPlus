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

#include <rpp/disposables/interface_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/disposables/callback_disposable.hpp>

#include <memory>

namespace rpp
{
struct interface_composite_disposable : public interface_disposable
{
    virtual void add(disposable_wrapper disposable) = 0;

    template<rpp::constraint::is_nothrow_invocable Fn>
    void add(Fn&& invocable)
    {
        add(make_callback_disposable(std::forward<Fn>(invocable)));
    }
};
}