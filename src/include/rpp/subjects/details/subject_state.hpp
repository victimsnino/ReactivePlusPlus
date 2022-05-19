//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/subscribers/dynamic_subscriber.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/overloaded.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <variant>
#include <vector>

namespace rpp::subjects::details
{
struct completed {};
struct unsubscribed {};

template<rpp::constraint::decayed_type T>
class subject_state : public std::enable_shared_from_this<subject_state<T>>
{
    using subscriber = dynamic_subscriber<T>;
    using shared_subscribers = std::shared_ptr<std::vector<subscriber>>;
    using state_t = std::variant<shared_subscribers, std::exception_ptr, completed, unsubscribed>;

public:
    subject_state() = default;

    void on_subscribe(const subscriber& subscriber)
    {
        std::unique_lock lock{m_mutex};

        process_state(m_state,
                      [&](shared_subscribers subs)
                      {
                          auto new_subs = make_copy_of_subscribed_subs(subs->size() + 1, subs);
                          new_subs->push_back(subscriber);
                          m_state = new_subs;

                          lock.unlock();

                          add_callback_on_unsubscribe(subscriber);
                      },
                      [&](std::exception_ptr err)
                      {
                          lock.unlock();
                          subscriber.on_error(err);
                      },
                      [&](completed)
                      {
                          lock.unlock();
                          subscriber.on_completed();
                      },
                      [&](unsubscribed)
                      {
                          lock.unlock();
                          subscriber.unsubscribe();
                      });
    }

    void on_next(const T& v)
    {
        std::unique_lock lock{m_mutex};

        process_state(m_state,
                      [&](shared_subscribers subs)
                      {
                          lock.unlock();

                          std::ranges::for_each(*subs, [&](const auto& sub) { sub.on_next(v); });
                      });
    }

    void on_error(const std::exception_ptr& err)
    {
        std::unique_lock lock{m_mutex};

        process_state(m_state,
                      [&](shared_subscribers subs)
                      {
                          m_state = err;
                          lock.unlock();

                          std::ranges::for_each(*subs, [&](const auto& sub) { sub.on_error(err); });
                      });
    }

    void on_completed()
    {
        std::unique_lock lock{m_mutex};

        process_state(m_state,
                      [&](shared_subscribers subs)
                      {
                          m_state = completed{};
                          lock.unlock();

                          std::ranges::for_each(*subs, std::mem_fn(&dynamic_subscriber<T>::on_completed));
                      });
    }

    void on_unsubscribe()
    {
        std::unique_lock lock{m_mutex};

        process_state(m_state,
                      [&](shared_subscribers subs)
                      {
                          m_state = unsubscribed{};
                          lock.unlock();

                          std::ranges::for_each(*subs, std::mem_fn(&dynamic_subscriber<T>::unsubscribe));
                      });
    }

private:
    static void process_state(const state_t& state, const auto&...actions)
    {
        std::visit(rpp::utils::overloaded{ actions..., [](auto) {} }, state);
    }

    static shared_subscribers make_copy_of_subscribed_subs(size_t expected_size, shared_subscribers current_subs)
    {
        auto subs = std::make_shared<std::vector<dynamic_subscriber<T>>>();
        subs->reserve(expected_size);
        std::ranges::copy_if(*current_subs,
                             std::back_inserter(*subs),
                             std::mem_fn(&dynamic_subscriber<T>::is_subscribed));
        return subs;
    }

    void add_callback_on_unsubscribe(const dynamic_subscriber<T>& subscriber)
    {
        auto weak = this->weak_from_this();
        subscriber.get_subscription().add([weak]
        {
            if (auto shared = weak.lock())
            {
                std::unique_lock lock{shared->m_mutex};
                process_state(shared->m_state,
                              [&](const shared_subscribers& subs)
                              {
                                  auto new_size = std::max(subs->size(), size_t{1}) - 1;
                                  shared->m_state = shared->make_copy_of_subscribed_subs(new_size, subs);
                              });
            }
        });
    }

private:
    std::mutex m_mutex{};
    state_t    m_state = std::make_shared<std::vector<subscriber>>();
};
} // namespace rpp::subjects::details
