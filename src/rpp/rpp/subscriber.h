// MIT License
// 
// Copyright (c) 2021 Aleksey Loginov
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

#include <rpp/observer.h>

#include <variant>

namespace rpp
{
template<typename Type>
class subscriber final
{
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type should be decayed");
public:
    template<typename TType, typename = std::enable_if_t<std::is_same_v<std::decay_t<TType>, Type>>>
    subscriber(const observer<TType>& observer)
        : m_observer{observer} { }

    template<typename TType, typename = std::enable_if_t<std::is_same_v<std::decay_t<TType>, Type>>>
    subscriber(observer<TType>&& observer)
        : m_observer{std::move(observer)} { }

    template<typename U, typename = std::enable_if_t<std::is_same_v<std::decay_t<U>, Type>>>
    void on_next(U&& val) const
    {
        std::visit([&](auto& obs) { obs.on_next(std::forward<U>(val)); }, m_observer);
    }
    void on_error(const error& err) const { std::visit([&](auto& obs) { obs.on_error(err); }, m_observer); }
    void on_completed() const { std::visit([&](auto& obs) { obs.on_completed(); }, m_observer); }

private:
    std::variant<observer<Type>,
                 observer<Type&>,
                 observer<const Type&>,
                 observer<Type&&>> m_observer;
};
} // namespace rpp
