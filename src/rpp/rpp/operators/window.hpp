#pragma once

#include <rpp/operators/fwd/window.hpp>
#include <rpp/subscribers/constraints.hpp>

#include <rpp/subjects/publish_subject.hpp>

IMPLEMENTATION_FILE(window_tag);

namespace rpp
{
template<constraint::decayed_type Type>
using windowed_observable = decltype(std::declval<subjects::publish_subject<Type>>().get_observable());
}

namespace rpp::details
{
template<constraint::decayed_type Type>
struct window_state
{
    const size_t                    window_size{};
    mutable size_t                  items_in_current_window = window_size;
    mutable subjects::publish_subject<Type> subject{};
};

struct window_on_next
{
    template<constraint::decayed_type Type>
    void operator()(auto&& value, const auto& subscriber, const window_state<Type>& state) const
    {
        // need to send new subject due to NEW item appeared (we avoid sending new subjects if no any new items)
        if (state.items_in_current_window == state.window_size)
        {
            subscriber.on_next(state.subject.get_observable());
            state.items_in_current_window = 0;
        }

        ++state.items_in_current_window;
        state.subject.get_subscriber().on_next(std::forward<decltype(value)>(value));

        // cleanup current subject, but don't send due to wait for new value
        if (state.items_in_current_window == state.window_size)
        {
            state.subject.get_subscriber().on_completed();
            state.subject = rpp::subjects::publish_subject<Type>{};
        }
    }
};

struct window_on_error
{
    template<constraint::decayed_type Type>
    void operator()(const std::exception_ptr& err, const auto& subscriber, const window_state<Type>& state) const
    {
        state.subject.get_subscriber().on_error(err);
        subscriber.on_error(err);
    }
};

struct window_on_completed
{
    template<constraint::decayed_type Type>
    void operator()(const auto& subscriber, const window_state<Type>& state) const
    {
        state.subject.get_subscriber().on_completed();
        subscriber.on_completed();
    }
};

template<constraint::decayed_type Type>
struct window_lift_impl
{
    size_t window_size{};

    template<constraint::subscriber_of_type<windowed_observable<Type>> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto subscription = subscriber.get_subscription();

        // dynamic_state there to make shared_ptr for observer instead of making shared_ptr for state
        return create_subscriber_with_dynamic_state<Type>(std::move(subscription),
                                                          window_on_next{},
                                                          window_on_error{},
                                                          window_on_completed{},
                                                          std::forward<TSub>(subscriber),
                                                          window_state<Type>{window_size});
    }
};

template<constraint::decayed_type Type, constraint::observable_of_type<Type> TObs>
auto window_impl(TObs&& obs, size_t window_size)
{
    return std::forward<TObs>(obs).template lift<windowed_observable<Type>>(window_lift_impl<Type>{window_size});
}
} // namespace rpp::details
