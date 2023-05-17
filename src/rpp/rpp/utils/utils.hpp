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

#include <rpp/defs.hpp>
#include <rpp/utils/constraints.hpp>

#include <concepts>
#include <iterator>

namespace rpp::utils {

struct none{};

template<constraint::iterable T>
using iterable_value_t = std::iter_value_t<decltype(std::begin(std::declval<T>()))>;

template<class T>
constexpr std::add_const_t<T>& as_const(const T& v) noexcept { return v; }

template<class T>
constexpr T&& as_const(T&& v) noexcept requires std::is_rvalue_reference_v<T&&> { return std::forward<T>(v); }

struct convertible_to_any
{
    convertible_to_any() = default;

    template<typename T>
    operator T();
};

/**
 * @brief Calls passed function during destruction
 */
template<std::invocable Fn>
class finally_action
{
public:
    explicit finally_action(Fn&& fn) : m_fn{std::move(fn)} {}

    explicit finally_action(const Fn& fn) : m_fn{fn} {}

    finally_action(const finally_action&)     = delete;
    finally_action(finally_action&&) noexcept = delete;

    ~finally_action() noexcept { m_fn(); }

private:
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;
};

template<rpp::constraint::decayed_type T>
class repeated_container
{
public:
    repeated_container(T&& value, size_t count) : m_value{std::move(value)}, m_count{count} {}
    repeated_container(const T& value, size_t count) : m_value{value}, m_count{count} {}

    class iterator
    {
    public:
        iterator(const repeated_container* container, size_t index) : m_container{container}, m_index{index} {}

        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;

        const value_type& operator*() const { return m_container->m_value; }
        iterator& operator++() { ++m_index; return *this; }
        iterator operator++(int) { auto old = *this; ++(*this); return old; }

        bool operator==(const iterator&) const = default;
        bool operator!=(const iterator&) const = default;

    private:
        const repeated_container* m_container;
        size_t m_index;
    };

    iterator begin() const { return {this, 0}; }
    iterator end() const { return {this, m_count}; }

private:
    T m_value;
    size_t m_count;
};
} // namespace rpp::utils
