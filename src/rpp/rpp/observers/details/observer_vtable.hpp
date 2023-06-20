//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/fwd.hpp>
#include <rpp/disposables/fwd.hpp>

namespace rpp::details::observers
{
template<typename T, typename Strategy>
void forwarding_on_next_lvalue(const void* const ptr, const T& v) { static_cast<const Strategy*>(ptr)->on_next(v); }

template<typename T, typename Strategy>
void forwarding_on_next_rvalue(const void* const ptr, T&& v) { static_cast<const Strategy*>(ptr)->on_next(std::forward<T>(v)); }

template<typename Strategy>
void forwarding_on_error(const void* const ptr, const std::exception_ptr& err) { static_cast<const Strategy*>(ptr)->on_error(err); }

template<typename Strategy>
void forwarding_on_completed(const void* const ptr) { static_cast<const Strategy*>(ptr)->on_completed(); }

template<typename Strategy>
void forwarding_set_upstream(void* const ptr, const disposable_wrapper& d) { static_cast<Strategy*>(ptr)->set_upstream(d); }

template<typename Strategy>
bool forwarding_is_disposed(const void* const ptr) { return static_cast<const Strategy*>(ptr)->is_disposed(); }

template<typename Strategy>
void forwarding_destroy_at(void* const ptr) { std::destroy_at(static_cast<Strategy*>(ptr)); }

template<typename Strategy>
void forwarding_move_to(void* const origin, void* const dest) { std::construct_at(static_cast<Strategy*>(dest), std::move(*static_cast<Strategy*>(origin))); }

template<constraint::decayed_type Type>
struct observer_vtable
{
    void (*on_next_lvalue)(const void*, const Type&){};
    void (*on_next_rvalue)(const void*, Type&&){};
    void (*on_error)(const void*, const std::exception_ptr&){};
    void (*on_completed)(const void*){};

    void (*set_upstream)(void*, const disposable_wrapper&){};
    bool (*is_disposed)(const void*){};

    void (*destroy_at)(void*){};
    void (*move_to)(void*, void*){};

    template<constraint::observer_strategy<Type> Strategy>
    static const observer_vtable* create() noexcept
    {
        static observer_vtable s_res{
            .on_next_lvalue = forwarding_on_next_lvalue<Type, Strategy>,
            .on_next_rvalue = forwarding_on_next_rvalue<Type, Strategy>,
            .on_error = forwarding_on_error<Strategy>,
            .on_completed = forwarding_on_completed<Strategy>,
            .set_upstream = forwarding_set_upstream<Strategy>,
            .is_disposed = forwarding_is_disposed<Strategy>,
            .destroy_at = forwarding_destroy_at<Strategy>,
            .move_to = forwarding_move_to<Strategy>
        };
        return &s_res;
    }
};
}