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

namespace rpp::utils
{
class spinlock
{
public:
    spinlock() = default;

    void lock()
    {
        while(m_lock_flag.exchange(true, std::memory_order_acquire))
        {
            while(m_lock_flag.load(std::memory_order_relaxed)){};
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
