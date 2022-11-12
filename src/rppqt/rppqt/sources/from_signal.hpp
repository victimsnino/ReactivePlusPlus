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

#include <rppqt/sources/fwd.hpp>
#include <rpp/subjects/publish_subject.hpp>

#include <QObject>

#include <tuple>

IMPLEMENTATION_FILE(from_signal_qt_tag);

namespace rppqt::observable::details
{
template<typename Value>
struct from_signal_on_event_base
{
    using Subject = rpp::subjects::publish_subject<Value>;
    using Subscriber = decltype(std::declval<Subject>().get_subscriber());

    from_signal_on_event_base(const Subscriber& subscriber)
        : subscriber{subscriber} {}

    from_signal_on_event_base(Subscriber&& subscriber)
        : subscriber{std::move(subscriber)} {}

    Subscriber subscriber;
};

template<typename... Args>
struct from_signal_on_event : from_signal_on_event_base<std::tuple<std::decay_t<Args>...>>
{
    template<typename ...Vals>
    void operator()(Vals&&... vals) const
    {
        from_signal_on_event_base<std::tuple<std::decay_t<Args>...>>::subscriber.on_next(std::make_tuple(std::forward<Vals>(vals)...));
    }
};

template<typename Arg>
struct from_signal_on_event<Arg> : from_signal_on_event_base<std::decay_t<Arg>>
{
    template<rpp::constraint::decayed_same_as<Arg> Val>
    void operator()(Val&& val) const
    {
        from_signal_on_event_base<std::decay_t<Arg>>::subscriber.on_next(std::forward<Val>(val));
    }
};

template<>
struct from_signal_on_event<> : from_signal_on_event_base<rpp::utils::none>
{
    void operator()() const
    {
        subscriber.on_next(rpp::utils::none{});
    }
};

}
namespace rppqt::observable
{
template<std::derived_from<QObject> TObject, typename R, typename... Args>
auto from_signal(const TObject& object, R (TObject::*signal)(Args ...)) requires rpp::details::is_header_included<rpp::details::from_signal_qt_tag, TObject, R, Args...>
{
    using on_next_impl = details::from_signal_on_event<Args...>;
    const auto subj = typename on_next_impl::Subject{};

    const auto subscriber     = subj.get_subscriber();
    QObject::connect(&object, signal, on_next_impl{subscriber});
    QObject::connect(&object, &QObject::destroyed, [subscriber] { subscriber.on_completed(); });

    return subj.get_observable();
}
} // namespace rppqt::observable