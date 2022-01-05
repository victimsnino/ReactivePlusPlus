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

#include <rpp/fwd.h>

#include <memory>

namespace rpp::details
{
template<typename SubscriberType>
struct iobservable_storage
{
    virtual      ~iobservable_storage() = default;
    virtual void on_subscribe(SubscriberType sub) const = 0;
};

template<typename Type>
class observable_state
{
    using subscriber_type = const subscriber<Type>&;

    template<typename T>
    using enable_if_is_callable_t = std::enable_if_t<std::is_invocable_v<T, subscriber_type>>;

public:
    template<typename OnSubscribe, typename = enable_if_is_callable_t<OnSubscribe>>
    observable_state(OnSubscribe&& on_subscribe)
        : m_storage{std::make_shared<storage<std::decay_t<OnSubscribe>>>(std::forward<OnSubscribe>(on_subscribe))} {}

    observable_state(const observable_state& other)     = default;
    observable_state(observable_state&& other) noexcept = default;
    ~observable_state()                                 = default;

    observable_state& operator=(const observable_state& other)     = default;
    observable_state& operator=(observable_state&& other) noexcept = default;

    void on_subscribe(const subscriber<Type>& subscriber) const
    {
        m_storage->on_subscribe(subscriber);
    }

private:
    template<typename OnSubscribe>
    class storage final : public iobservable_storage<subscriber_type>
    {
    public:
        storage(OnSubscribe&& on_subscribe)
            : m_on_subscribe{std::move(on_subscribe)} {}

        storage(const OnSubscribe& on_subscribe)
            : m_on_subscribe{on_subscribe} {}

        storage(const storage& other)                = delete;
        storage(storage&& other) noexcept            = delete;
        storage& operator=(const storage& other)     = delete;
        storage& operator=(storage&& other) noexcept = delete;

        void on_subscribe(subscriber_type sub) const override
        {
            m_on_subscribe(sub);
        }

    private:
        const OnSubscribe m_on_subscribe;
    };

    std::shared_ptr<const iobservable_storage<subscriber_type>> m_storage;
};
} // namespace rpp::details
