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

#include <catch2/catch_test_macros.hpp>
#include <rpp/utils/shared_on_copy.h>

#include <functional>

TEST_CASE("shared_on_copy base checks", "[shared_on_copy]")
{
    copy_count_tracker tracker{};

    WHEN("Construct shared_on_copy by copy")
    {
        rpp::utils::shared_on_copy shared{tracker};
        THEN("Only one copy")
        {
            CHECK(tracker.get_copy_count() == 1);
            CHECK(tracker.get_move_count() == 0);
        }
        AND_WHEN("Make copy of shared")
        {
            [[maybe_unused]] auto shared_copy = shared.clone();
            THEN("Make one more move into shared_ptr")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 1);
            }
            AND_WHEN("Any new copies of shared")
            {
                [[maybe_unused]] auto another_shared_copy = shared.clone();
                [[maybe_unused]] auto copy_of_shared_copy = shared_copy.clone();
                THEN("No any new copies due to shared_ptr")
                {
                    CHECK(tracker.get_copy_count() == 1);
                    CHECK(tracker.get_move_count() == 1);
                }
            }
        }
        AND_WHEN("Make move of shared")
        {
            [[maybe_unused]] auto shared_copy = shared.move();
            THEN("Make one more move, but not into shared_ptr")
            {
                CHECK(tracker.get_copy_count() == 1);
                CHECK(tracker.get_move_count() == 1);
            }
            AND_WHEN("Any new copies of shared")
            {
                [[maybe_unused]] auto another_shared_copy = shared_copy.clone();
                THEN("Extra move to shared_ptr")
                {
                    CHECK(tracker.get_copy_count() == 1);
                    CHECK(tracker.get_move_count() == 2);
                }
            }
        }
    }
    WHEN("Construct shared_on_copy by move")
    {
        rpp::utils::shared_on_copy shared{std::move(tracker)};
        THEN("Only one move")
        {
            CHECK(tracker.get_copy_count() == 0);
            CHECK(tracker.get_move_count() == 1);
        }
    }
}
