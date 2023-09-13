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

/**
 * @defgroup disposables Disposables
 * @brief Disposable owns some resource and provides ability to `dispose()` it: destroy/remove/disconnect and etc.
 * @details In RPP it used as "reverse subscription": observable sets disposable to observer via `set_upstream(disposable)` with meaning "if you want to cancel me -> dispose this disposable"
 * @ingroup rpp
 */

#include <rpp/disposables/fwd.hpp>

#include <rpp/disposables/callback_disposable.hpp>
#include <rpp/disposables/composite_disposable.hpp>
#include <rpp/disposables/disposable_wrapper.hpp>
#include <rpp/disposables/interface_disposable.hpp>
#include <rpp/disposables/refcount_disposable.hpp>
