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

namespace rpp::details
{
template<typename Type>
struct iobserver_storage
{
private:
    static_assert(std::is_same_v<std::decay_t<Type>, Type>, "Type passed to iobserver_storage should be decayed!");
public:
    virtual      ~iobserver_storage() = default;
    virtual void on_next(Type&) const = 0;
    virtual void on_next(const Type&) const = 0;
    virtual void on_next(Type&&) const = 0;
    virtual void on_error(std::exception_ptr) const = 0;
    virtual void on_completed() const = 0;
};

template<typename Type, typename OnNext, typename OnError, typename OnCompleted>
class observer_storage final : public iobserver_storage<std::decay_t<Type>>
{
    using Decayed = std::decay_t<Type>;
public:
    template<typename TOnNext, typename TOnError, typename TOnCompleted>
    observer_storage(TOnNext&& on_next, TOnError&& on_error, TOnCompleted&& on_completed)
        : m_on_next{std::forward<TOnNext>(on_next)}
        , m_on_error{std::forward<TOnError>(on_error)}
        , m_on_completed{std::forward<TOnCompleted>(on_completed)} {}

    observer_storage(const observer_storage& other)                = delete;
    observer_storage(observer_storage&& other) noexcept            = delete;
    observer_storage& operator=(const observer_storage& other)     = delete;
    observer_storage& operator=(observer_storage&& other) noexcept = delete;

    void on_next(Decayed& val) const override
    {
        if constexpr (!std::is_rvalue_reference_v<Type>)
            m_on_next(val);
        else
            throw std::logic_error("Can't convert lvalue reference to rvalue reference");
    }

    void on_next(const Decayed& val) const override
    {
        if constexpr (!std::is_rvalue_reference_v<Type> && !std::is_same_v<Decayed&, Type>)
            m_on_next(val);
        else
            throw std::logic_error("Can't send const lvalue reference to rvalue or non-const lvalue reference");
    }

    void on_next(Decayed&& val) const override
    {
        if constexpr (!std::is_same_v<Type, Decayed&>)
            m_on_next(std::move(val));
        else
            throw std::logic_error("Can't send rvalue reference to non-const lvalue reference");
    }

    void on_error(std::exception_ptr err) const override
    {
        m_on_error(err);
    }

    void on_completed() const override
    {
        m_on_completed();
    }

private:
    const OnNext      m_on_next;
    const OnError     m_on_error;
    const OnCompleted m_on_completed;
};

template<typename Type>
class observer_state
{
    using Decayed = std::decay_t<Type>;

    template<typename T>
    static constexpr bool is_type_v = std::is_same_v<T, Type>;
public:
    template<typename OnNext, typename OnError, typename OnCompleted>
    observer_state(OnNext&& on_next, OnError&& on_error, OnCompleted&& on_completed)
        : m_storage{std::make_shared<observer_storage<Type, OnNext, OnError, OnCompleted>>(std::forward<OnNext>(on_next),
                                                                                           std::forward<OnError>(on_error),
                                                                                           std::forward<OnCompleted>(on_completed))} {}

    observer_state(const observer_state& other)     = default;
    observer_state(observer_state&& other) noexcept = default;
    ~observer_state() noexcept                      = default;

    observer_state& operator=(const observer_state& other)     = default;
    observer_state& operator=(observer_state&& other) noexcept = default;

    template<typename U>
    void on_next(U&& val) const
    {
        static constexpr bool s_is_forwardable = is_type_v<Decayed> || is_type_v<const Decayed> || is_type_v<const Decayed&>;
        static constexpr bool s_is_ref = is_type_v<Decayed&>;
        static constexpr bool s_is_rvalue = is_type_v<Decayed&&> || is_type_v<const Decayed&&>;
        if constexpr (s_is_forwardable)
        {
            m_storage->on_next(std::forward<U>(val));
        }
        else if constexpr (s_is_ref)
        {
            if constexpr (std::is_same_v<U, Decayed&> || std::is_same_v<U, Decayed>)
                m_storage->on_next(val);
            else
            {
                Decayed temp{std::forward<U>(val)};
                m_storage->on_next(temp);
            }
        }
        else if constexpr (s_is_rvalue)
        {
            if constexpr (std::is_lvalue_reference_v<U>)
                m_storage->on_next(Decayed{val});
            else
                m_storage->on_next(std::forward<U>(val));
        }
        else
            static_assert(!s_is_forwardable && !s_is_ref && !s_is_rvalue, "Some unsupported type detected!");
    }

    void on_error(std::exception_ptr err) const
    {
        m_storage->on_error(err);
    }

    void on_completed() const
    {
        m_storage->on_completed();
    }

private:
    std::shared_ptr<const iobserver_storage<Decayed>> m_storage;
};
} // namespace rpp::details
