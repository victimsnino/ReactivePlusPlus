// MIT License
// 
// Copyright (c) 2022 Aleksey Loginov
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <memory>
#include <shared_mutex>
#include <variant>

namespace rpp::utils
{
template<typename Type>
class shared_on_copy
{
public:
    shared_on_copy() = default;

    template<typename U, typename = std::enable_if_t<std::is_same_v<std::decay_t<Type>, std::decay_t<U>>>>
    shared_on_copy(U&& val)
        : m_data{std::forward<U>(val)} {}

    shared_on_copy(const shared_on_copy& other)                = delete;
    shared_on_copy(shared_on_copy&& other) noexcept            = delete;
    shared_on_copy& operator=(const shared_on_copy& other)     = delete;
    shared_on_copy& operator=(shared_on_copy&& other) noexcept = delete;

    shared_on_copy clone() const
    {
        std::unique_lock lock{m_mutex};
        if (std::holds_alternative<Type>(m_data))
            m_data = std::make_shared<Type>(std::move(std::get<Type>(m_data)));

        return shared_on_copy{m_data};
    }

    shared_on_copy move()
    {
        std::unique_lock lock{m_mutex};
        return shared_on_copy{std::move(m_data)};
    }

    template<typename F, typename = std::enable_if_t<std::is_invocable_v<F, Type&>>>
    auto apply(const F& f) const
    {
        std::shared_lock guard{m_mutex};
        if (auto* data = std::get_if<Type>(&m_data))
            return f(*data);

        return f(*std::get<std::shared_ptr<Type>>(m_data));
    }

private:
    shared_on_copy(const std::variant<Type, std::shared_ptr<Type>>& data)
        : m_data{data} {}

    shared_on_copy(std::variant<Type, std::shared_ptr<Type>>&& data)
        : m_data{std::move(data)} {}

private:
    mutable std::variant<Type, std::shared_ptr<Type>> m_data{std::in_place_type_t<Type>{}};
    mutable std::shared_mutex                         m_mutex;
};

template<typename Type>
shared_on_copy(Type) -> shared_on_copy<Type>;
} // namespace rpp::utils
