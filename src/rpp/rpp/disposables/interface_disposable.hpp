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
#include <memory>

namespace rpp
{
using disposable_ptr = std::shared_ptr<interface_disposable>;

struct interface_disposable
{
    virtual ~interface_disposable() noexcept = default;

    virtual bool is_disposed() const noexcept = 0;
    virtual void dispose()                    = 0;
};
}