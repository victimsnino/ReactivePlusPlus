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

#include <rpp/utils/function_traits.h>

#include <memory>
#include <utility>

namespace rpp
{
struct error;

namespace detail
{
    template<typename ...Type>
    struct EmptyFunctor
    {
        void operator()(const Type&...) const {}
    };

    template<typename Type>
    struct iobserver
    {
        virtual      ~iobserver() = default;
        virtual void on_next(Type val) const = 0;
        virtual void on_error(const error& err) const = 0;
        virtual void on_completed() const = 0;
    };

    template<typename Type>
    class observer_state final : iobserver<Type>
    {
    public:
        template<typename OnNext, typename OnError, typename OnCompleted>
        observer_state(OnNext&& on_next, OnError&& on_error, OnCompleted&& on_completed)
            : m_storage{std::make_shared<StorageType<OnNext, OnError, OnCompleted>>(
             std::forward<OnNext>(on_next),
             std::forward<OnError>(on_error),
             std::forward<OnCompleted>(on_completed))}
            , m_on_next{[](void* storage, Type val)
            {
                ToStoragePtr<OnNext, OnError, OnCompleted>(storage)->on_next(std::forward<Type>(val));
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

        void on_next(Type val) const override { m_on_next(m_storage.get(), std::forward<Type>(val)); }
        void on_error(const error& err) const override { m_on_error(m_storage.get(), err); }
        void on_completed() const override { m_on_completed(m_storage.get()); }

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

        using OnNextFn = void(*)(void*, Type);
        using OnErrorFn = void(*)(void*, const error&);
        using OnCompleted = void(*)(void*);
    private:
        std::shared_ptr<void> m_storage;

        const OnNextFn    m_on_next;
        const OnErrorFn   m_on_error;
        const OnCompleted m_on_completed;
    };
} //namespace detail

template<typename Type>
class observer final : public detail::iobserver<Type>
{
public:
    template<typename OnNext = detail::EmptyFunctor<Type>,
             typename OnError = detail::EmptyFunctor<error>,
             typename OnCompleted = detail::EmptyFunctor<>,
             typename = std::enable_if_t<std::is_invocable_v<OnNext, Type>>>
    observer(OnNext&& on_next = {}, OnError&& on_error = {}, OnCompleted&& on_completed = {})
        : m_observer_state{std::forward<OnNext>(on_next),
                           std::forward<OnError>(on_error),
                           std::forward<OnCompleted>(on_completed)} {}

    observer(const observer&)     = default;
    observer(observer&&) noexcept = default;

    void on_next(Type val) const override { m_observer_state.on_next(std::forward<Type>(val)); }
    void on_error(const error& err) const override { m_observer_state.on_error(err); }
    void on_completed() const override { m_observer_state.on_completed(); }

private:
    detail::observer_state<Type> m_observer_state;
};

template<typename OnNext>
observer(OnNext) -> observer<utils::function_argument_t<OnNext>>;
}
