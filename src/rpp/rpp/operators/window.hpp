#pragma once

#include <rpp/operators/fwd/window.hpp>
#include <rpp/subscribers/constraints.hpp>

#include <rpp/subjects/publish_subject.hpp>

IMPLEMENTATION_FILE(window_tag);

namespace rpp
{
template<constraint::decayed_type Type>
using windowed_observable = decltype(std::declval<rpp::subjects::publish_subject<Type>>().get_observable());
}

namespace rpp::details
{
template<constraint::decayed_type Type, constraint::subscriber TSub>
class window_observer final : public details::typed_observer_tag<Type>
{
    struct state_t
    {
        state_t(const TSub& subscriber, size_t window_size)
            : subscriber{subscriber}
            , window_size{window_size}
            , items_in_current_window{window_size} {}

        state_t(TSub&& subscriber, size_t window_size)
            : subscriber{ std::move(subscriber)}
            , window_size{ window_size }
            , items_in_current_window{ window_size } {}

        TSub                                 subscriber;

        const size_t                         window_size{};
        size_t                               items_in_current_window = window_size;

        rpp::subjects::publish_subject<Type> subject{};
    };

public:
    window_observer(const TSub& subscriber, size_t window_size)
        : m_state{ std::make_shared<state_t>(subscriber, window_size)} {}

    window_observer(TSub&& subscriber, size_t window_size)
        : m_state{ std::make_shared<state_t>(std::move(subscriber), window_size) } {}

    void on_next(auto&& v) const { on_next_impl(std::forward<decltype(v)>(v)); }
    void on_error(const std::exception_ptr& err) const { broadcast([&err](const auto& sub) { sub.on_error(err); }); }
    void on_completed() const { broadcast([](const auto& sub) { sub.on_completed(); }); }

private:
    void on_next_impl(auto&& val) const
    {
        if (m_state->items_in_current_window == m_state->window_size)
        {
            m_state->subscriber.on_next(m_state->subject.get_observable());
            m_state->items_in_current_window = 0;
        }

        ++m_state->items_in_current_window;
        m_state->subject.get_subscriber().on_next(std::forward<decltype(val)>(val));

        if (m_state->items_in_current_window == m_state->window_size)
        {
            m_state->subject.get_subscriber().on_completed();
            m_state->subject = rpp::subjects::publish_subject<Type>{};
        }
    }

    void broadcast(const auto& action) const
    {
        action(m_state->subject.get_subscriber());
        action(m_state->subscriber);
    }

    std::shared_ptr<state_t> m_state;
};

template<constraint::decayed_type Type>
struct window_lift_impl
{
    size_t window_size{};

    template<constraint::subscriber_of_type<windowed_observable<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        return rpp::specific_subscriber<Type, window_observer<Type, std::decay_t<TSub>>>(std::move(subscription), std::forward<TSub>(subscriber), window_size);
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto window_impl(TObs&& obs, size_t window_size)
{
    return std::forward<TObs>(obs).template lift<windowed_observable<Type>>(window_lift_impl<Type>{window_size});
}
} // namespace rpp::details
