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

#include "rpp/subscription.h"

#include <rpp/schedulers/fwd.h>
#include <rpp/schedulers/constraints.h>

#include <chrono>
#include <concepts>
#include <thread>

namespace rpp::schedulers
{
class immediate final : public details::scheduler_tag
{
public:
    class worker
    {
    public:
        worker(rpp::subscription sub)
            : m_sub{std::move(sub)} {}

        void schedule(const constraint::schedulable_fn auto& fn)
        {
            schedule(std::chrono::high_resolution_clock::now(), fn);
        }

        void schedule(time_point time_point, const constraint::schedulable_fn auto& fn)
        {
            while (m_sub.is_subscribed())
            {
                std::this_thread::sleep_until(time_point);
                auto duration = fn();
                if (!duration.has_value())
                    return;
                time_point += duration.value();
            }
        }
    private:
        rpp::subscription m_sub;
    };

    static worker create_worker(rpp::subscription sub = {})
    {
        return worker{ sub };
    }
};
} // namespace rpp::schedulers
