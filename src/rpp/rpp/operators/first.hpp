//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2022 - present.
//                            TC Wang 2022 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/fwd/first.hpp>
#include <rpp/operators/take.hpp>
#include <rpp/subscribers/constraints.hpp>
#include <rpp/utils/exceptions.hpp>
#include <rpp/utils/functors.hpp>
#include <rpp/utils/utilities.hpp>

#include <memory>

IMPLEMENTATION_FILE(first_tag);

namespace rpp::details
{

template<constraint::decayed_type Type>
struct first_impl
{
private:
    struct first_value_forwarder
    {
        void forward_on_next(auto&& value, const constraint::subscriber auto& subscriber) const
        {
           m_take_on_next(std::forward<decltype(value)>(value), subscriber);
        }

        void forward_on_completed(const constraint::subscriber auto& subscriber) const
        {
            if (m_take_on_next.get_emission_count_left() != 0)
            {
                auto err = std::make_exception_ptr(rpp::utils::not_enough_emissions{"'first' operator should at least emit the value once."});
                subscriber.on_error(err);
            }
        }

    private:
        rpp::details::take_on_next m_take_on_next{1};
    };

public:
    template<constraint::subscriber_of_type<Type> TSub>
    auto operator()(TSub&& subscriber) const
    {
        auto forwarder = std::make_shared<first_value_forwarder>();

        auto on_next = [forwarder](auto&& value, const auto& subscriber)
        {
            forwarder->forward_on_next(std::forward<decltype(value)>(value), subscriber);
        };
        auto on_completed = [forwarder](const auto& subscriber)
        {
            forwarder->forward_on_completed(subscriber);
        };

        auto subscription = subscriber.get_subscription();
        return create_subscriber_with_state<Type>(std::move(subscription),
                                                  std::move(on_next),
                                                  utils::forwarding_on_error{},
                                                  std::move(on_completed),
                                                  std::forward<TSub>(subscriber));
    }
};

} // namespace rpp::details
