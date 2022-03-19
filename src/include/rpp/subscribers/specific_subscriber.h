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
#include <rpp/observers/specific_observer.h>
#include <rpp/subscribers/subscriber_base.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/type_traits.h>

namespace rpp
{
/**
 * \brief specific version of subscriber which stores type of observer used inside to prevent extra allocations
 * \tparam Type type of values expected by this subscriber
 * \tparam Observer observer which was wrapped by this subscriber
 */
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
    template<typename ...Types>
        requires utils::is_observer_constructible_v<Type, Types...>
    specific_subscriber(Types&&...vals)
        : specific_subscriber{subscription{}, std::forward<Types>(vals)...} {}

    template<typename ...Types>
        requires utils::is_observer_constructible_v<Type, Types...>
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

template<constraint::observer TObs>
specific_subscriber(TObs observer) -> specific_subscriber<utils::extract_observer_type_t<TObs>, TObs>;

template<constraint::observer TObs>
specific_subscriber(subscription, TObs observer) -> specific_subscriber<utils::extract_observer_type_t<TObs>, TObs>;

template<typename OnNext,
         typename ...Args,
         typename Type = std::decay_t<utils::function_argument_t<OnNext>>>
specific_subscriber(subscription, OnNext, Args ...) -> specific_subscriber<Type, details::deduce_specific_observer_type_t<OnNext, Args...>>;

template<typename OnNext,
         typename ...Args,
         typename Type = std::decay_t<utils::function_argument_t<OnNext>>>
specific_subscriber(OnNext, Args ...) -> specific_subscriber<Type, details::deduce_specific_observer_type_t<OnNext, Args...>>;


/**
 * \brief Creation of rpp::specific_subscriber with manual providing of type of subscriber. In case of ability to determine type of subscriber by function -> use constructor
 */
template<typename Type, typename ...Args>
auto make_specific_subscriber(Args&& ...args)
{
    auto observer = rpp::make_specific_observer<Type>(std::forward<Args>(args)...);
    return rpp::specific_subscriber<Type, decltype(observer)>(std::move(observer));
}

template<typename Type, typename ...Args>
auto make_specific_subscriber(const subscription& sub, Args&& ...args)
{
    auto observer = rpp::make_specific_observer<Type>(std::forward<Args>(args)...);
    return rpp::specific_subscriber<Type, decltype(observer)>(sub, std::move(observer));
}

template<typename Type, typename ...Args>
auto make_specific_subscriber(subscription&& sub, Args&& ...args)
{
    auto observer = rpp::make_specific_observer<Type>(std::forward<Args>(args)...);
    return rpp::specific_subscriber<Type, decltype(observer)>(std::move(sub), std::move(observer));
}

} // namespace rpp