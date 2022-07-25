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

#include <atomic>
#include <memory>
#include <type_traits>
#include <optional>

namespace rpp::utils
{
template<class T>
constexpr std::add_const_t<T>& as_const(const T& v) noexcept { return v; }

template<class T>
constexpr T&& as_const(T&& v) noexcept requires std::is_rvalue_reference_v<T&&> { return std::forward<T>(v); }

#if __cpp_lib_atomic_shared_ptr
template<typename T>
using atomic_shared_ptr = std::atomic<std::shared_ptr<T>>;
#else
template<typename T>
using atomic_shared_ptr = std::shared_ptr<T>;
#endif

// used as interpetation of "void"
struct none{};


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
    [[no_unique_address]] std::optional<Callable> m_callable;
};
} // namespace rpp::utils
