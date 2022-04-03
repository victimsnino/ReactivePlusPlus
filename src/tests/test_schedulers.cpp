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

#include <catch2/catch_test_macros.hpp>
#include <rpp/schedulers/immediate_scheduler.h>

SCENARIO("Immediate scheduler schedule task immediately")
{
    GIVEN("immediate_scheduler")
    {
        auto scheduler = rpp::schedulers::immediate{};

        auto worker = scheduler.create_worker();
        WHEN("scheduler without time and resheduling")
        {
            size_t call_count{};
            worker.schedule([&call_count]() -> rpp::schedulers::optional_duration {++call_count; return {}; });
            THEN("called once immediately")
            {
                CHECK(call_count == 1);
            }
        }
        WHEN("scheduler without time with scheduling")
        {
            size_t call_count{};
            worker.schedule([&call_count]() -> rpp::schedulers::optional_duration
            {
                if (++call_count <= 1)
                    return {{}};
                return {};
            });
            THEN("called twice immediately")
            {
                CHECK(call_count == 2);
            }
		}
		WHEN("scheduler with time")
		{
			size_t call_count{};
            auto now = rpp::schedulers::clock_type::now();
            rpp::schedulers::time_point execute_time{};
			worker.schedule(rpp::schedulers::clock_type::now() + std::chrono::seconds{ 2 },
				[&call_count, &execute_time]() -> rpp::schedulers::optional_duration
				{
					++call_count;
                    execute_time = rpp::schedulers::clock_type::now();
					return {};
				});
			THEN("called twice immediately")
			{
				REQUIRE(call_count == 1);
                REQUIRE(execute_time - now >= std::chrono::seconds{ 2 });
			}
		}
        WHEN("scheduler without time with scheduling")
        {
            size_t                                   call_count{};
            std::vector<rpp::schedulers::time_point> executions{};
            worker.schedule([&call_count, &executions]() -> rpp::schedulers::optional_duration
                {
                    executions.push_back(rpp::schedulers::clock_type::now());
                    if (++call_count <= 1)
                        return { std::chrono::seconds{3} };
                    return {};
                });
            THEN("called twice immediately")
            {
                REQUIRE(call_count == 2);
                REQUIRE(executions[1] - executions[0] >= std::chrono::seconds{ 3 });
            }
        }
	}
}


SCENARIO("Immediate scheduler depends on subscription")
{
    GIVEN("immediate_scheduler")
    {
        auto scheduler = rpp::schedulers::immediate{};
        WHEN("pass unsubscribed subscription")
        {
            rpp::subscription sub{};
            sub.unsubscribe();
            auto worker = scheduler.create_worker(sub);

            size_t call_count{};
            worker.schedule([&call_count]() -> rpp::schedulers::optional_duration
            {
                ++call_count;
                return {{}};
            });
            THEN("no any calls/schedules")
            {
                CHECK(call_count == 0);
            }
        }
        WHEN("unsubscribe during function")
        {
            rpp::subscription sub{};
            auto worker = scheduler.create_worker(sub);

            size_t call_count{};
            worker.schedule([&call_count, sub]() -> rpp::schedulers::optional_duration
                {
                    if (++call_count > 1)
                        sub.unsubscribe();
                    return { {} };
                });
            THEN("no any calls/schedules after unsubscribe")
            {
                CHECK(call_count == 2);
            }
        }
    }
}
