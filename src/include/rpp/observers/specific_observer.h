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
#include <rpp/observers/dynamic_observer.h>
#include <rpp/observers/interface_observer.h>
#include <rpp/utils/function_traits.h>
#include <rpp/utils/functors.h>

namespace rpp
{
/**
 * \brief Observer specified with specific template types of callbacks to avoid extra heap usage.
 *
 * \details It has better performance comparing to rpp::dynamic_observer due to using stack instead of heap as long as possible. It is default return type of all operators
 *
 * \tparam T is type of value handled by this observer
 * \tparam OnNext type of on_next callback
 * \tparam OnError type of on_error callback
 * \tparam OnCompleted type of on_completed callback
 */
template<constraint::decayed_type T,
         constraint::on_next_fn<T>   OnNext      = utils::empty_function_t<T>,
         constraint::on_error_fn     OnError     = utils::empty_function_t<std::exception_ptr>,
         constraint::on_completed_fn OnCompleted = utils::empty_function_t<>>
class specific_observer final : public interface_observer<T>
{
public:
    specific_observer() = default;

    specific_observer(constraint::on_next_fn<T> auto&& on_next)
        : m_on_next{std::forward<decltype(on_next)>(on_next)} {}

    specific_observer(constraint::on_next_fn<T> auto&& on_next, constraint::on_completed_fn auto&& on_completed)
        : m_on_next{std::forward<decltype(on_next)>(on_next)}
        , m_on_completed{std::forward<decltype(on_completed)>(on_completed)} {}

    specific_observer(constraint::on_next_fn<T> auto&& on_next, constraint::on_error_fn auto&& on_error)
        : m_on_next{std::forward<decltype(on_next)>(on_next)}
        , m_on_err{std::forward<decltype(on_error)>(on_error)} {}

    specific_observer(constraint::on_next_fn<T> auto&&   on_next,
                      constraint::on_error_fn auto&&     on_error,
                      constraint::on_completed_fn auto&& on_completed)
        : m_on_next{std::forward<decltype(on_next)>(on_next)}
        , m_on_err{std::forward<decltype(on_error)>(on_error)}
        , m_on_completed{std::forward<decltype(on_completed)>(on_completed)} {}

    specific_observer(const constraint::observer auto& obs)
        requires (!std::is_same_v<std::decay_t<decltype(obs)>, specific_observer<T, OnNext, OnError, OnCompleted>>)
        : m_on_next{utils::make_forwarding_on_next(obs)}
        , m_on_err{utils::make_forwarding_on_error(obs)}
        , m_on_completed{utils::make_forwarding_on_completed(obs)} {}

    specific_observer(const specific_observer<T, OnNext, OnError, OnCompleted>& other)     = default;
    specific_observer(specific_observer<T, OnNext, OnError, OnCompleted>&& other) noexcept = default;

    void on_next(const T& v) const override { m_on_next(v); }
    void on_next(T&& v) const override { m_on_next(std::move(v)); }
    void on_error(const std::exception_ptr& err) const override { m_on_err(err); }
    void on_completed() const override { m_on_completed(); }

    /**
    * \brief Converting current rpp::specific_observer to rpp::dynamic_observer alternative with erasing of type (and using heap)
    * \details This overload makes copy of rpp::specific_observer to rpp::dynamic_observer
    * \return converted rpp::dynamic_observer
    */
    [[nodiscard]] dynamic_observer<T> as_dynamic() const & { return *this; }
    /**
     *
     * \brief Converting current rpp::specific_observer to rpp::dynamic_observer alternative with erasing of type (and using heap)
     * \details This overload makes copy of rpp::specific_observer to rpp::dynamic_observer
     * \return converted rpp::dynamic_observer
     */
    [[nodiscard]] dynamic_observer<T> as_dynamic() && { return std::move(*this); }

private:
    OnNext      m_on_next{};
    OnError     m_on_err{};
    OnCompleted m_on_completed{};
};

template<typename OnNext>
specific_observer(OnNext) -> specific_observer<utils::decayed_function_argument_t<OnNext>, OnNext>;

template<typename OnNext, constraint::on_error_fn OnError, typename ...Args>
specific_observer(OnNext  ,
                  OnError ,
                  Args ...) -> specific_observer<utils::decayed_function_argument_t<OnNext>, OnNext, OnError, Args...>;

template<typename OnNext, constraint::on_completed_fn OnCompleted>
specific_observer(OnNext     ,
                  OnCompleted) -> specific_observer<
    utils::decayed_function_argument_t<OnNext>, OnNext, utils::empty_function_t<std::exception_ptr>, OnCompleted>;

template<constraint::observer TObs>
specific_observer(TObs) -> specific_observer<utils::extract_observer_type_t<TObs>,
                                             decltype(utils::make_forwarding_on_next(std::declval<TObs>())),
                                             decltype(utils::make_forwarding_on_error(std::declval<TObs>())),
                                             decltype(utils::make_forwarding_on_completed(std::declval<TObs>()))>;


template<typename...Args>
using decayed_specific_observer = rpp::specific_observer<std::decay_t<Args>...>;

/**
 * \brief Create specific_observer with manually specified Type. In case of type can be deduced from argument of OnNext use direct constructor of rpp::specific_observer
 * \tparam Type manually specific type of observer
 */
template<typename Type>
auto make_specific_observer() -> decayed_specific_observer<Type>
{
    return {};
};

/**
 * \brief Create specific_observer with manually specified Type. In case of type can be deduced from argument of OnNext use direct constructor of rpp::specific_observer
 * \tparam Type manually specific type of observer
 */
template<typename Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next) -> decayed_specific_observer<Type, decltype(on_next)>
{
    return {std::forward<decltype(on_next)>(on_next)};
}

/**
 * \brief Create specific_observer with manually specified Type. In case of type can be deduced from argument of OnNext use direct constructor of rpp::specific_observer
 * \tparam Type manually specific type of observer
 */
template<typename Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next,
                            constraint::on_completed_fn auto&&  on_completed) -> decayed_specific_observer<Type, decltype(on_next), utils::empty_function_t<std::exception_ptr>, decltype(on_completed)>
{
    return {std::forward<decltype(on_next)>(on_next), std::forward<decltype(on_completed)>(on_completed)};
}

/**
 * \brief Create specific_observer with manually specified Type. In case of type can be deduced from argument of OnNext use direct constructor of rpp::specific_observer
 * \tparam Type manually specific type of observer
 */
template<typename Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next,
                            constraint::on_error_fn auto&&      on_error) -> decayed_specific_observer<Type, decltype(on_next), decltype(on_error)>
{
    return {std::forward<decltype(on_next)>(on_next), std::forward<decltype(on_error)>(on_error)};
}

/**
 * \brief Create specific_observer with manually specified Type. In case of type can be deduced from argument of OnNext use direct constructor of rpp::specific_observer
 * \tparam Type manually specific type of observer
 */
template<typename Type>
auto make_specific_observer(constraint::on_next_fn<Type> auto&& on_next,
                            constraint::on_error_fn auto&&      on_error,
                            constraint::on_completed_fn auto&&  on_completed) -> decayed_specific_observer<Type, decltype(on_next), decltype(on_error), decltype(on_completed)>
{
    return {std::forward<decltype(on_next)>(on_next),
            std::forward<decltype(on_error)>(on_error),
            std::forward<decltype(on_completed)>(on_completed)};
}


namespace details
{
    template<typename Type, typename ...Args>
    using deduce_specific_observer_type_t = decltype(make_specific_observer<Type>(std::declval<Args>()...));
} // namespace details
} // namespace rpp
