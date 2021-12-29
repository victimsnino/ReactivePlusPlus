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
template<typename Type>
class observable_state
{
    using subscriber_type = const subscriber<Type>&;

    template<typename T>
    using enable_if_is_callable_t = std::enable_if_t<std::is_invocable_v<T, subscriber_type>>;

public:
    template<typename OnSubscribe, typename = enable_if_is_callable_t<OnSubscribe>>
    observable_state(OnSubscribe&& on_subscribe)
        : m_storage{std::make_shared<StorageType<OnSubscribe>>(std::forward<OnSubscribe>(on_subscribe))}
        , m_on_subscribe{[](void* storage, subscriber_type subscriber)
        {
            ToStoragePtr<OnSubscribe>(storage)->on_subscribe(subscriber);
        }} {}

    observable_state(const observable_state&)     = default;
    observable_state(observable_state&&) noexcept = default;

    void on_subscribe(const subscriber<Type>& subscriber) const
    {
        m_on_subscribe(m_storage.get(), subscriber);
    }

private:
    template<typename OnSubscribe>
    struct Storage
    {
        template<typename TOnSubscribe, typename = enable_if_is_callable_t<TOnSubscribe>>
        Storage(TOnSubscribe&& on_subscribe)
            : on_subscribe{std::forward<TOnSubscribe>(on_subscribe)} {}

        const OnSubscribe on_subscribe;
    };

    template<typename OnSubscribe>
    using StorageType = Storage<std::decay_t<OnSubscribe>>;

    template<typename OnSubscribe>
    static auto ToStoragePtr(void* ptr) { return static_cast<StorageType<OnSubscribe>*>(ptr); }

    using OnSubscribeFn = void(*)(void*, subscriber_type);
private:
    std::shared_ptr<void> m_storage;
    const OnSubscribeFn   m_on_subscribe;
};
} // namespace rpp::details
