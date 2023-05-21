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
class disposable_guard
{
public:
    disposable_guard(std::shared_ptr<base_disposable> disposable = {}) : m_disposable{std::move(disposable)} {}

private:
    std::shared_ptr<base_disposable> m_disposable;
};
}