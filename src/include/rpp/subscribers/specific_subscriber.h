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

#include <rpp/fwd.h>
#include <rpp/subscribers/subscriber_base.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/type_traits.h>

namespace rpp
{
template<typename Type, typename Observer>
class specific_subscriber : public details::subscriber_base<Type>
{
    static_assert(std::is_same_v<std::decay_t<Observer>, Observer>, "Observer should be decayed");
public:
    //********************* Construct by observer *********************//
    specific_subscriber(const Observer& observer)
        : specific_subscriber{subscription{}, observer} { }

    specific_subscriber(Observer&& observer)
        : specific_subscriber{subscription{}, std::move(observer)} { }

    specific_subscriber(const subscription& sub, const Observer& observer)
        : details::subscriber_base<Type>{sub}
        , m_observer{observer} { }

    specific_subscriber(const subscription& sub, Observer&& observer)
        : details::subscriber_base<Type>{sub}
        , m_observer{std::move(observer)} { }

    //********************* Construct by actions *********************//
    template<typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    specific_subscriber(Types&&...vals)
        : specific_subscriber{subscription{}, std::forward<Types>(vals)...} {}

    template<typename ...Types,
             typename Enabled = utils::enable_if_observer_constructible_t<Type, Types...>>
    specific_subscriber(const subscription& sub, Types&&...vals)
        : details::subscriber_base<Type>{sub}
        , m_observer{std::forward<Types>(vals)...} {}

    // ************* Copy/Move ************************* //
    specific_subscriber(const specific_subscriber&)     = default;
    specific_subscriber(specific_subscriber&&) noexcept = default;

    const Observer& get_observer() const
    {
        return m_observer;
    }

    auto as_dynamic() const & { return dynamic_subscriber<Type>{*this};            }
    auto as_dynamic() &&      { return dynamic_subscriber<Type>{std::move(*this)}; }
protected:
    void on_next_impl(const Type& val) const final
    {
        m_observer.on_next(val);
    }

    void on_next_impl(Type&& val) const final
    {
        m_observer.on_next(std::move(val));
    }

    void on_error_impl(const std::exception_ptr& err) const final
    {
        m_observer.on_error(err);
    }

    void on_completed_impl() const final
    {
        m_observer.on_completed();
    }

private:
    Observer m_observer;
};

template<typename T>
specific_subscriber(dynamic_observer<T> observer) -> specific_subscriber<T, dynamic_observer<T>>;

template<typename T, typename ...Args>
specific_subscriber(specific_observer<T, Args...> observer) -> specific_subscriber<T, specific_observer<T, Args...>>;

template<typename TSub,
         typename OnNext,
         typename ...Args,
         typename = std::enable_if_t<utils::is_callable_v<OnNext> && std::is_same_v<TSub, subscription>>,
         typename Type = std::decay_t<utils::function_argument_t<OnNext>>>
specific_subscriber(TSub, OnNext, Args ...) -> specific_subscriber<Type, specific_observer<Type, OnNext, Args...>>;

template<typename OnNext,
         typename ...Args,
         typename = std::enable_if_t<utils::is_callable_v<OnNext>>,
         typename Type = std::decay_t<utils::function_argument_t<OnNext>>>
specific_subscriber(OnNext, Args ...) -> specific_subscriber<Type, specific_observer<Type, OnNext, Args...>>;

} // namespace rpp