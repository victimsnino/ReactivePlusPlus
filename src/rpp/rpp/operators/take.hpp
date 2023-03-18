//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd.hpp>
#include <rpp/observables/interface_observable.hpp>
#include <rpp/observers/details/operator_observer.hpp>
#include <type_traits>

namespace rpp::details
{
struct take_strategy final : public forwarding_on_error_strategy
                          , public forwarding_on_completed_strategy
{
    mutable size_t count;

    template<constraint::decayed_type DownStreamType>
    void on_next(const interface_observer<DownStreamType>& obs, constraint::decayed_same_as<DownStreamType> auto&& v) const noexcept
    {
        if (count > 0)
        {
            --count;
            obs.on_next(std::forward<decltype(v)>(v));
        }

        if (count == 0)
            obs.on_completed();
    }
};
}

namespace rpp::operators
{
template<constraint::decayed_type T, constraint::observable_of_type<T> TObs>
class take_observable final : public interface_observable<T>
{
public:
    take_observable(size_t count, constraint::decayed_same_as<TObs> auto&& obs)
        : m_count(count)
        , m_obs(std::forward<decltype(obs)>(obs))
        {}

protected:
    composite_disposable subscribe_impl(interface_observer<T>&& observer) const noexcept override
    {
        return m_obs.subscribe(details::operator_observer<T, T, details::take_strategy>{{m_count}, observer});
    }
private:
    size_t m_count;
    TObs m_obs;
};

class take {
public:
    take(size_t count) : m_count{count} {}

    template<constraint::observable TObs>
    take_observable<utils::extract_observable_type_t<TObs>, std::decay_t<TObs>> operator()(TObs&& observable) {
        return {m_count, std::forward<TObs>(observable)};
    }

private:
    size_t m_count;
};
} // namespace rpp::operators
