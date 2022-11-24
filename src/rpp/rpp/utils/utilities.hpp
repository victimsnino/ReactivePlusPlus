//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>

#include <atomic>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <optional>

namespace rpp::utils
{
template<class T>
constexpr std::add_const_t<T>& as_const(const T& v) noexcept { return v; }

template<class T>
constexpr T&& as_const(T&& v) noexcept requires std::is_rvalue_reference_v<T&&> { return std::forward<T>(v); }

#if defined(__cpp_lib_atomic_shared_ptr) && __cpp_lib_atomic_shared_ptr
template<typename T>
using atomic_shared_ptr = std::atomic<std::shared_ptr<T>>;
#else
template<typename T>
using atomic_shared_ptr = std::shared_ptr<T>;
#endif

// used as interpetation of "void"
struct none{};

template<constraint::iterable T>
using iterable_value_t = typename decltype(std::begin(std::declval<T>()))::value_type;

template<typename Cont, std::invocable<iterable_value_t<Cont>> Fn>
void for_each(Cont&& container, Fn&& fn)
{
    std::for_each(std::begin(container), std::end(container), std::forward<Fn>(fn));
}

template<typename Cont, std::predicate<iterable_value_t<Cont>> Fn>
bool all_of(const Cont& container, const Fn& fn)
{
    return std::all_of(std::cbegin(container), std::cend(container), fn);
}

// some objects can't be copy/moved-assigned, but can be copy/move constructed (like immutable lambdas), so, use trick with optional and emplace.
template<typename Callable>
class copy_assignable_callable
{
public:
    copy_assignable_callable(const Callable& callable)
        : m_callable{callable} {}

    copy_assignable_callable(Callable&& callable)
        : m_callable{std::move(callable)} {}

    ~copy_assignable_callable() = default;

    copy_assignable_callable(const copy_assignable_callable& other)
        : m_callable{other.m_callable} {}

    copy_assignable_callable(copy_assignable_callable&& other) noexcept (std::is_nothrow_move_constructible_v<Callable>)
        : m_callable{std::move(other.m_callable)} {}

    copy_assignable_callable& operator=(const copy_assignable_callable& other)
    {
        if (this == &other)
            return *this;
        if (other.m_callable.has_value())
            m_callable.emplace(other.m_callable.value());
        else
            m_callable.reset();
        return *this;
    }

    copy_assignable_callable& operator=(copy_assignable_callable&& other) noexcept
    {
        if (this == &other)
            return *this;
        if (other.m_callable.has_value())
            m_callable.emplace(std::move(other.m_callable).value());
        else
            m_callable.reset();
        return *this;
    }

    decltype(auto) operator()(auto&&...args) { return (*m_callable)(std::forward<decltype(args)>(args)...); }
    decltype(auto) operator()(auto&&...args) const { return (*m_callable)(std::forward<decltype(args)>(args)...); }

private:
    RPP_NO_UNIQUE_ADDRESS std::optional<Callable> m_callable;
};

/**
 * \brief Calls passed function during destruction
 */
template<std::invocable Fn>
class finally_action
{
public:
    finally_action(Fn&& fn)
        : m_fn{std::move(fn)} {}

    finally_action(const Fn& fn)
        : m_fn{fn} {}

    ~finally_action() noexcept
    {
        m_fn();
    }
private:
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;
};
} // namespace rpp::utils
