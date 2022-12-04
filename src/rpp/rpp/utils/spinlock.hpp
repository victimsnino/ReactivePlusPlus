//                   ReactivePlusPlus library
// 
//           Copyright Aleksey Loginov 2022 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
// 
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <atomic>
#include <thread>

namespace rpp::utils
{
class spinlock
{
public:
    spinlock() = default;

    void lock()
    {
        while (m_lock_flag.exchange(true, std::memory_order_acq_rel))
        {
            for (uint8_t i = 0; m_lock_flag.load(std::memory_order_relaxed); ++i)
            {
                if (i == 30u)
                {
                    std::this_thread::yield();
                    i = 0;
                }
            }
        }
    }

    void unlock()
    {
        m_lock_flag.store(false, std::memory_order_release);
    }

private:
    std::atomic_bool m_lock_flag{false};
};
} // namespace rpp::utils
