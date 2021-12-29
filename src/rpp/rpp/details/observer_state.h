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

#include <memory>

namespace rpp
{
struct error;
} // namespace rpp

namespace rpp::details
{
template<typename ...Type>
struct EmptyFunctor
{
    void operator()(const Type&...) const {}
};

template<typename Type>
class observer_state final
{
    using Decayed = std::decay_t<Type>;
public:
    template<typename OnNext, typename OnError, typename OnCompleted>
    observer_state(OnNext&& on_next, OnError&& on_error, OnCompleted&& on_completed)
        : m_storage{std::make_shared<StorageType<OnNext, OnError, OnCompleted>>(
                                                                                std::forward<OnNext>(on_next),
                                                                                std::forward<OnError>(on_error),
                                                                                std::forward<
                                                                                    OnCompleted>(on_completed))}
        , m_on_next_ref{[](void* storage, Decayed& val)
        {
            if constexpr (!std::is_rvalue_reference_v<Type>)
                ToStoragePtr<OnNext, OnError, OnCompleted>(storage)->on_next(val);
            else
                throw std::logic_error("Can't send lvalue reference to rvalue reference");
        }}, m_on_next_const_ref{[](void* storage, const Decayed& val)
        {
            if constexpr (!std::is_rvalue_reference_v<Type> && !std::is_same_v<Decayed&, Type>)
                ToStoragePtr<OnNext, OnError, OnCompleted>(storage)->on_next(val);
            else
                throw std::logic_error("Can't send lvalue reference to rvalue reference");
        }}
        , m_on_next_move{[](void* storage, Decayed&& val)
        {
            if constexpr (!std::is_same_v<Type, Decayed&>)
                ToStoragePtr<OnNext, OnError, OnCompleted>(storage)->on_next(std::move(val));
            else
                throw std::logic_error("Can't send rvalue reference to nonconst lvalue reference");
        }}
        , m_on_error{[](void* storage, const error& err)
        {
            ToStoragePtr<OnNext, OnError, OnCompleted>(storage)->on_error(err);
        }}
        , m_on_completed{[](void* storage)
        {
            ToStoragePtr<OnNext, OnError, OnCompleted>(storage)->on_completed();
        }} {}

    observer_state(const observer_state&)     = default;
    observer_state(observer_state&&) noexcept = default;

    template<typename U>
    void on_next(U&& val) const
    {
        if constexpr (std::is_same_v<Decayed, Type> || std::is_same_v<const Decayed, Type>) // T || const T
        {
            if constexpr (std::is_lvalue_reference_v<U>)
                m_on_next_const_ref(m_storage.get(), val);
            else
                m_on_next_move(m_storage.get(), std::forward<U>(val));
        }
        else if constexpr (std::is_same_v<const Decayed&, Type>) // const T&
            m_on_next_const_ref(m_storage.get(), val);
        else if constexpr (std::is_same_v<Decayed&, Type>) // T&
        {
            if constexpr (std::is_same_v<U, Decayed&> || std::is_same_v<U, Decayed>)
                m_on_next_ref(m_storage.get(), val);
            else
            {
                Decayed temp{std::forward<U>(val)};
                m_on_next_ref(m_storage.get(), temp);
            }
        }
        else if constexpr (std::is_same_v<Decayed&&, Type> || std::is_same_v<const Decayed&&, Type>) // T&& || const T&&
        {
            if constexpr (std::is_lvalue_reference_v<U>)
                m_on_next_move(m_storage.get(), Decayed{val});
            else
                m_on_next_move(m_storage.get(), std::forward<U>(val));
        }
        else
            throw std::logic_error("Some unsupported type detected!");
    }

    void on_error(const error& err) const { m_on_error(m_storage.get(), err); }
    void on_completed() const { m_on_completed(m_storage.get()); }

private:
    template<typename OnNext, typename OnError, typename OnCompleted>
    struct Storage
    {
        template<typename TOnNext, typename TOnError, typename TOnCompleted>
        Storage(TOnNext&& on_next, TOnError&& on_error, TOnCompleted&& on_completed)
            : on_next{std::forward<TOnNext>(on_next)}
            , on_error{std::forward<TOnError>(on_error)}
            , on_completed{std::forward<TOnCompleted>(on_completed)} {}

        const OnNext      on_next;
        const OnError     on_error;
        const OnCompleted on_completed;
    };

    template<typename OnNext, typename OnError, typename OnCompleted>
    using StorageType = Storage<std::decay_t<OnNext>, std::decay_t<OnError>, std::decay_t<OnCompleted>>;

    template<typename OnNext, typename OnError, typename OnCompleted>
    static auto ToStoragePtr(void* ptr) { return static_cast<StorageType<OnNext, OnError, OnCompleted>*>(ptr); }

    using OnNextRefFn = void(*)(void*, Decayed&);
    using OnNextConstRefFn = void(*)(void*, const Decayed&);
    using OnNextMoveFn = void(*)(void*, Decayed&&);

    using OnErrorFn = void(*)(void*, const error&);
    using OnCompleted = void(*)(void*);
private:
    std::shared_ptr<void> m_storage;

    const OnNextRefFn      m_on_next_ref;
    const OnNextConstRefFn m_on_next_const_ref;
    const OnNextMoveFn     m_on_next_move;

    const OnErrorFn   m_on_error;
    const OnCompleted m_on_completed;
};
} // namespace rpp::details
