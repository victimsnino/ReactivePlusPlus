//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/defs.hpp>
#include <rpp/sources/fwd.hpp>
#include <rpp/observables/base_observable.hpp>
#include <rpp/utils/utils.hpp>
#include <rpp/schedulers/current_thread.hpp>
#include <rpp/operators/map.hpp>

#include <array>
#include <exception>
#include <memory>
#include <utility>

namespace rpp::details
{
template<constraint::decayed_type Container>
class shared_container
{
public:
    template<typename ...Ts>
        requires (!constraint::variadic_decayed_same_as<shared_container<Container>, Ts...>)
    explicit shared_container(Ts&&...items)
        // raw "new" call to avoid extra copy/moves for items
        : m_container{new Container{std::forward<Ts>(items)...}} {}

    shared_container(const shared_container&) = default;
    shared_container(shared_container&&) noexcept = default;

    auto begin() const { return std::cbegin(*m_container); }
    auto end() const { return std::cend(*m_container); }

    auto get_actual_iterator() const
    {
        if (!m_iterator)
            m_iterator = begin();
        return m_iterator.value();
    }
    bool increment_iterator() const
    {
        if (!m_iterator)
            m_iterator = begin();

        return ++(m_iterator.value()) != end();
    }

    static const auto& extract_value_from_itr(const auto& itr) {
        return *itr;
    }

private:
    std::shared_ptr<Container>                                 m_container{};
    mutable std::optional<decltype(std::cbegin(*m_container))> m_iterator;
};

template<constraint::decayed_type Container>
class container_with_iterator
{
public:
    template<typename ...Ts>
        requires (!constraint::variadic_decayed_same_as<container_with_iterator<Container>, Ts...>)
    explicit container_with_iterator(Ts&&...items)
        : m_container{std::forward<Ts>(items)...} {}

    container_with_iterator(const container_with_iterator& other)
        : m_container{other.m_container}
        , m_index(other.m_index)
    {}

    container_with_iterator(container_with_iterator&& other) noexcept
        : m_container{std::move(other.m_container)}
        , m_index(other.m_index)
    {}

    auto begin() const { return std::cbegin(m_container); }
    auto end() const { return std::cend(m_container); }

    auto get_actual_iterator() const
    {
        if (!m_iterator)
            m_iterator = get_default_iterator_value();
        return m_iterator.value();
    }
    bool increment_iterator() const
    {
        if (!m_iterator)
            m_iterator = get_default_iterator_value();

        ++m_index;
        return ++(m_iterator.value()) != end();
    }

    static auto extract_value_from_itr(const auto& itr) {
        return std::move(*itr);
    }

private:
    auto get_default_iterator_value() const
    {
        auto itr = begin();
        std::advance(itr, m_index);
        return itr;
    }

private:
    RPP_NO_UNIQUE_ADDRESS Container                           m_container{};
    mutable size_t                                            m_index{};
    mutable std::optional<decltype(std::cbegin(m_container))> m_iterator{};
};

template<constraint::memory_model memory_model, constraint::iterable Container, typename ...Ts>
auto pack_to_container(Ts&& ...items)
{
    if constexpr (std::same_as<memory_model, rpp::memory_model::use_stack>)
        return container_with_iterator<Container>{std::forward<Ts>(items)...};
    else
        return shared_container<Container>{std::forward<Ts>(items)...};
}

template<constraint::memory_model memory_model, constraint::decayed_type T, typename ...Ts>
auto pack_variadic(Ts&& ...items)
{
    return pack_to_container<memory_model, std::array<T, sizeof...(Ts)>>(std::forward<Ts>(items)...);
}

template<constraint::decayed_type PackedContainer, schedulers::constraint::scheduler TScheduler>
struct from_iterable_strategy
{
    RPP_NO_UNIQUE_ADDRESS PackedContainer container;
    RPP_NO_UNIQUE_ADDRESS TScheduler      scheduler;

    template<constraint::observer_strategy<utils::iterable_value_t<PackedContainer>> Strategy>
    void subscribe(base_observer<utils::iterable_value_t<PackedContainer>, Strategy>&& observer) const
    {
        if constexpr (std::same_as<TScheduler, schedulers::immediate>)
        {
            try
            {
                for (const auto& v : container)
                {
                    if (observer.is_disposed())
                        return;

                    observer.on_next(v);
                }

                observer.on_completed();
            }
            catch (...)
            {
                observer.on_error(std::current_exception());
            }
        }
        else
        {
            const auto worker = scheduler.create_worker();
            observer.set_upstream(worker.get_disposable());
            worker.schedule([](const base_observer<utils::iterable_value_t<PackedContainer>, Strategy>& obs, const PackedContainer& cont) -> rpp::schedulers::optional_duration
            {
                try
                {
                    if (const auto itr = cont.get_actual_iterator(); itr != std::cend(cont))
                    {
                        obs.on_next(utils::as_const(*itr));
                        if (cont.increment_iterator()) // it was not last
                            return schedulers::duration{}; // re-schedule this
                    }

                    obs.on_completed();
                }
                catch(...)
                {
                    obs.on_error(std::current_exception());
                }
                return std::nullopt;
            }, std::move(observer), container);
        }
    }
};

template<typename PackedContainer, schedulers::constraint::scheduler TScheduler>
auto make_from_iterable_observable(PackedContainer&& container, const TScheduler& scheduler)
{
    return base_observable<utils::iterable_value_t<std::decay_t<PackedContainer>>,
                           details::from_iterable_strategy<std::decay_t<PackedContainer>, TScheduler>>{std::forward<PackedContainer>(container),
                                                                                                       scheduler};
}
} // namespace rpp::details

namespace rpp::source
{
/**
 * @brief Creates observable that emits a items from provided iterable
 *
 * @marble from_iterable
   {
       operator "from_iterable({1,2,3,5})": +-1-2-3-5-|
   }
 *
 * @tparam memory_model rpp::memory_model strategy used to handle provided iterable
 * @param scheduler is scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * @param iterable container with values which will be flattened
 *
 * @par Examples:
 * @snippet from.cpp from_iterable
 * @snippet from.cpp from_iterable with model
 * @snippet from.cpp from_iterable with scheduler
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/from.html
 */
template<constraint::memory_model memory_model/* = memory_model::use_stack*/, constraint::iterable Iterable, schedulers::constraint::scheduler TScheduler /* = schedulers::current_thread*/>
auto from_iterable(Iterable&& iterable, const TScheduler& scheduler /* = TScheduler{}*/)
{
    return details::make_from_iterable_observable(details::pack_to_container<memory_model, std::decay_t<Iterable>>(std::forward<Iterable>(iterable)), scheduler);
}

/**
 * @brief Creates rpp::base_observable that emits a particular items and completes
 *
 * @marble just
   {
       operator "just(1,2,3,5)": +-1-2-3-5-|
   }
 *
 * @tparam memory_model rpp::memory_model startegy used to handle provided items
 * @tparam Scheduler type of scheduler used for scheduling of submissions: next item will be submitted to scheduler when previous one is executed
 * @param item first value to be sent
 * @param items rest values to be sent
 * @return rpp::base_observable which emits provided items
 *
 * @par Examples:
 * @snippet just.cpp just
 * @snippet just.cpp just memory model
 * @snippet just.cpp just scheduler
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/just.html
 */
template<constraint::memory_model memory_model /* = memory_model::use_stack */, schedulers::constraint::scheduler TScheduler, typename T, typename ...Ts>
auto just(const TScheduler& scheduler, T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...)
{
    return details::make_from_iterable_observable(details::pack_variadic<memory_model, std::decay_t<T>>(std::forward<T>(item), std::forward<Ts>(items)...), scheduler);
}

/**
 * @brief Creates rpp::base_observable that emits a particular items and completes
 * @warning this overloading uses trampoline scheduler as default
 *
 * @marble just
   {
       operator "just(1,2,3,5)": +-1-2-3-5-|
   }
 *
 * @tparam memory_model rpp::memory_model strategy used to handle provided items
 * @param item first value to be sent
 * @param items rest values to be sent
 * @return rpp::base_observable which emits provided items
 *
 * @par Examples:
 * @snippet just.cpp just
 * @snippet just.cpp just memory model
 * @snippet just.cpp just scheduler
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/just.html
 */
template<constraint::memory_model memory_model /* = memory_model::use_stack */, typename T, typename ...Ts>
auto just(T&& item, Ts&& ...items) requires (constraint::decayed_same_as<T, Ts> && ...)
{
    return details::make_from_iterable_observable(details::pack_variadic<memory_model, std::decay_t<T>>(std::forward<T>(item), std::forward<Ts>(items)...), schedulers::current_thread{});
}

/**
 * @brief Creates rpp::specific_observable that calls provided callable and emits resulting value of this callable
 *
 * @marble from_callable
   {
       operator "from_callable: [](){return 42;}": +-(42)--|
   }
 *
 * @tparam memory_model rpp::memory_model strategy used to handle callable
 *
 * @par Example
 * @snippet from.cpp from_callable
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/from.html
 */
template<constraint::memory_model memory_model /* = memory_model::use_stack */, std::invocable<> Callable>
auto from_callable(Callable&& callable)
{
    auto obs = just<memory_model>(std::forward<Callable>(callable));

    if constexpr (std::same_as<utils::decayed_invoke_result_t<Callable>, void>)
        return std::move(obs) | rpp::operators::map([](auto&& fn) { fn(); return utils::none{};});
    else
        return std::move(obs) | rpp::operators::map([](auto&& fn) { return fn(); });
}
}
