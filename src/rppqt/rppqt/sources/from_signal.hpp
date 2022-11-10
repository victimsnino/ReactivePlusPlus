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

IMPLEMENTATION_FILE(from_event_qt_tag);

namespace rppqt::observable::details
{
template<typename Value>
struct from_event_on_event_base
{
    using Subject = rpp::subjects::publish_subject<Value>;
    using Subscriber = decltype(std::declval<Subject>().get_subscriber());

    from_event_on_event_base(const Subscriber& subscriber)
        : subscriber{subscriber} {}

    from_event_on_event_base(Subscriber&& subscriber)
        : subscriber{std::move(subscriber)} {}

    Subscriber subscriber;
};

template<typename... Args>
struct from_event_on_event : from_event_on_event_base<std::tuple<std::decay_t<Args>...>>
{
    using from_event_on_event_base<std::tuple<std::decay_t<Args>...>>::from_event_on_event_base;

    template<typename ...Vals>
    void operator()(Vals&&... vals) const
    {
        from_event_on_event_base<std::tuple<std::decay_t<Args>...>>::subscriber.on_next(std::make_tuple(std::forward<Vals>(vals)...));
    }
};

template<typename Arg>
struct from_event_on_event<Arg> : from_event_on_event_base<std::decay_t<Arg>>
{
    using from_event_on_event_base<std::decay_t<Arg>>::from_event_on_event_base;

    template<rpp::constraint::decayed_same_as<Arg> Val>
    void operator()(Val&& val) const
    {
        from_event_on_event_base<std::decay_t<Arg>>::subscriber.on_next(std::forward<Val>(val));
    }
};

template<>
struct from_event_on_event<> : from_event_on_event_base<rpp::utils::none>
{
    using from_event_on_event_base<rpp::utils::none>::from_event_on_event_base;

    void operator()() const
    {
        subscriber.on_next(rpp::utils::none{});
    }
};

}
namespace rppqt::observable
{
template<std::derived_from<QObject> TObject, typename R, typename... Args>
auto from_event(const TObject& object, R (TObject::*signal)(Args ...)) requires rpp::details::is_header_included<rpp::details::from_event_qt_tag, TObject, R, Args...>
{
    using on_next_impl = details::from_event_on_event<Args...>;
    const auto subj = typename on_next_impl::Subject{};

    const auto subscriber     = subj.get_subscriber();
    auto event_conn     = QObject::connect(&object, signal, on_next_impl{subscriber});
    auto completed_conn = QObject::connect(&object, &QObject::destroyed, [subscriber] { subscriber.on_completed(); });

    subscriber.get_subscription()
              .add([ event_conn = std::move(event_conn), completed_conn = std::move(completed_conn) ]
              {
                  QObject::disconnect(event_conn);
                  QObject::disconnect(completed_conn);
              });

    return subj.get_observable();
}
} // namespace rppqt::observable