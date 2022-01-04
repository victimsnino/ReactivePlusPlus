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
#include <optional>
#include <shared_mutex>

namespace rpp::utils
{
/**
 * \brief class with idea similar to Copy-On-Write, but in this case we make shared_ptr on copy.
 *
 * Idea is pretty simple: if only 1 instance of this instance -> it uses stack to store data.
 * If someone tries to copy data -> we move data to shared ptr to avoid any future copies and provide synchronization between data
 * Underhood it uses shared_mutex to guarantee that no-one would change object during copy/move. But any user's modifications is under "shared_lock".
 *
 * This hack allows us to avoid using heap as long as possible to improve perfomance.
 * \tparam Type of data to store inside
 */
template<typename Type>
class shared_on_copy
{
public:
    shared_on_copy()
        : m_data{std::in_place_t{}} {}

    template<typename U, typename = std::enable_if_t<std::is_same_v<std::decay_t<Type>, std::decay_t<U>>>>
    shared_on_copy(U&& val)
        : m_data{std::forward<U>(val)} {}

    shared_on_copy(const shared_on_copy& other)                = delete;
    shared_on_copy(shared_on_copy&& other) noexcept            = delete;
    shared_on_copy& operator=(const shared_on_copy& other)     = delete;
    shared_on_copy& operator=(shared_on_copy&& other) noexcept = delete;

    /**
     * \brief Make copy of this storage to another one. Checks and updates storage if we need to move it from stack to shared_ptr
     */
    shared_on_copy clone() const
    {
        update_data(m_data, m_shared_data, m_mutex);
        return shared_on_copy{m_shared_data};
    }

    /**
     * \brief Just moves data to another instance under mutex
     */
    shared_on_copy move()
    {
        std::unique_lock lock{m_mutex};
        return shared_on_copy{std::move(m_data), std::move(m_shared_data)};
    }

    template<typename F, typename = std::enable_if_t<std::is_invocable_v<F, Type&>>>
    auto apply(const F& f) const
    {
        std::shared_lock guard{m_mutex};
        if (m_data.has_value())
            return f(m_data.value());

        return f(*m_shared_data);
    }

private:
    shared_on_copy(const std::shared_ptr<Type>& shared_data)
        : m_shared_data{shared_data} {}

    shared_on_copy(std::optional<Type>&& data, std::shared_ptr<Type>&& shared_data)
        : m_data{std::move(data)}
        , m_shared_data{std::move(shared_data)} {}

    static void update_data(std::optional<Type>& data, std::shared_ptr<Type>& shared_data, std::shared_mutex& mutex)
    {
        std::unique_lock lock{mutex};
        if (data.has_value())
        {
            shared_data = std::make_shared<Type>(std::move(data.value()));
            data.reset();
        }
    }

private:
    mutable std::optional<Type>   m_data{};
    mutable std::shared_ptr<Type> m_shared_data{};
    mutable std::shared_mutex     m_mutex;
};

template<typename Type>
shared_on_copy(Type) -> shared_on_copy<Type>;
} // namespace rpp::utils
