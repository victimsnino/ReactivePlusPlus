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

#include <mutex>

namespace rpp::operators::details
{
template<typename T>
struct value_with_mutex
{
    value_with_mutex() = default;
    explicit value_with_mutex(const T& v) : value{v} {}
    explicit value_with_mutex(T&& v) : value{std::move(v)} {}

    T          value{};
    std::mutex mutex{};
};

template<typename T>
class pointer_under_lock
{
public:
    explicit pointer_under_lock(value_with_mutex<T>& value)
        : pointer_under_lock{value.value, value.mutex}
    {}

    pointer_under_lock(T& val, std::mutex& mutex) : m_ptr{&val}, m_lock{mutex} {}

    T*       operator->() { return m_ptr; }
    const T* operator->() const { return m_ptr; }

private:
    T*                           m_ptr;
    std::scoped_lock<std::mutex> m_lock;
};
} // namespace rpp::operators::details