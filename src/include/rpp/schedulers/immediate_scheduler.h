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

#include <rpp/schedulers/constraints.h>
#include <rpp/schedulers/fwd.h>
#include <rpp/schedulers/worker.h>
#include <rpp/subscriptions/subscription_base.h>

#include <chrono>
#include <thread>

namespace rpp::schedulers
{
/**
 * \brief immediately calls provided schedulable or waits for time_point (in the caller-thread)
 */
class immediate final : public details::scheduler_tag
{
public:
    class worker_strategy
    {
    public:
        worker_strategy(const rpp::subscription_base& sub)
            : m_sub{sub} {}

        void schedule(time_point time_point, const constraint::schedulable_fn auto& fn) const
        {
            while (m_sub.is_subscribed())
            {
                std::this_thread::sleep_until(time_point);
                if (auto duration = fn())
                    time_point += duration.value();
                else
                    return;
            }
        }

    private:
        rpp::subscription_base m_sub;
    };

    static worker<worker_strategy> create_worker(const rpp::subscription_base& sub = {})
    {
        return worker<worker_strategy>{sub};
    }
};
} // namespace rpp::schedulers
