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

#include "copy_count_tracker.h"
#include "mock_observer.h"

#include <rpp/sources/from.h>
#include <catch2/catch_test_macros.hpp>

SCENARIO("from iterable")
{
    auto mock = mock_observer<int>();
    GIVEN("observable from iterable")
    {
        auto vals = std::vector{1, 2, 3, 4, 5, 6};
        auto obs  = rpp::source::from(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe(mock);
            THEN("observer obtains values in the same order")
            {
                CHECK(mock.get_received_values() == vals);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

SCENARIO("from iterable", "[track_copy]")
{
    copy_count_tracker tracker{};
    auto               vals         = std::array{tracker};
    auto               initial_copy = tracker.get_copy_count();
    auto               initial_move = tracker.get_move_count();

    GIVEN("observable from copied iterable")
    {
        auto obs = rpp::source::from(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to wrapped container
                CHECK(tracker.get_move_count() - initial_move == 2); // 1 move to lambda, 1 move lambda to observable
            }
        }
    }
    GIVEN("observable from moved iterable")
    {
        auto obs = rpp::source::from(std::move(vals));
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 0); 
                CHECK(tracker.get_move_count() - initial_move == 3); // 1 move to wrapped container + 1 move to lambda, 1 move lambda to observable
            }
        }
    }

    GIVEN("observable from copied iterable with shared memory model")
    {
        auto obs = rpp::source::from<rpp::memory_model::use_shared>(vals);
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 1); // 1 copy to shared_ptr
                CHECK(tracker.get_move_count() - initial_move == 0);
            }
        }
    }
    GIVEN("observable from moved iterable")
    {
        auto obs = rpp::source::from<rpp::memory_model::use_shared>(std::move(vals));
        WHEN("subscribe on it")
        {
            obs.subscribe();
            THEN("no extra copies")
            {
                CHECK(tracker.get_copy_count() - initial_copy == 0);
                CHECK(tracker.get_move_count() - initial_move == 1); // 1 copy to shared_ptr
            }
        }
    }
}
