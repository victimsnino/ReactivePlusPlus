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

#include <rpp/utils/constraints.h>
#include <rpp/observers/constraints.h>
#include <rpp/observers/interface_observer.h>

#include <tuple>

namespace rpp::details
{
template<typename OnNext, typename OnError, typename OnCompleted>
struct specific_callables
{
    const specific_callables* operator->() const { return this; }

    OnNext      on_next{};
    OnError     on_error{};
    OnCompleted on_completed{};
};

template<constraint::decayed_type T, typename Callables>
class base_observer : public interface_observer<T>
{
public:
    base_observer(constraint::on_next_fn<T> auto&& on_next)
        : m_callables{std::forward<decltype(on_next)>(on_next)} {}

    base_observer(constraint::on_next_fn<T> auto&&   on_next,
                  constraint::on_completed_fn auto&& on_completed)
        : m_callables{std::forward<decltype(on_next)>(on_next),
                      {},
                      std::forward<decltype(on_completed)>(on_completed)} {}

    base_observer(constraint::on_next_fn<T> auto&& on_next,
                  constraint::on_error_fn auto&&   on_error)
        : m_callables{std::forward<decltype(on_next)>(on_next), std::forward<decltype(on_error)>(on_error)} {}

    base_observer(constraint::on_next_fn<T> auto&&   on_next,
                  constraint::on_error_fn auto&&     on_error,
                  constraint::on_completed_fn auto&& on_completed)
        : m_callables{std::forward<decltype(on_next)>(on_next),
                      std::forward<decltype(on_error)>(on_error),
                      std::forward<decltype(on_completed)>(on_completed)} {}

    base_observer(std::same_as<Callables> auto&& callables)
        : m_callables{std::forward<decltype(callables)>(callables)} {}

    base_observer(const base_observer<T, Callables>& other)     = default;
    base_observer(base_observer<T, Callables>&& other) noexcept = default;

    void on_next(const T& v) const final
    {
        m_callables->on_next(v);
    }

    void on_next(T&& v) const final
    {
        m_callables->on_next(std::move(v));
    }

    void on_error(const std::exception_ptr& err) const final
    {
        m_callables->on_error(err);
    }

    void on_completed() const final
    {
        m_callables->on_completed();
    }

private:
    Callables m_callables{};
};
} // namespace rpp::details
