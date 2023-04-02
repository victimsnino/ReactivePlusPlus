#include "rpp/observers/fwd.hpp"
#include "rpp/sources/fwd.hpp"
#include <nanobench.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string_view>

#include <rpp/rpp.hpp>
#ifdef RPP_BUILD_RXCPP
    #include <rxcpp/rx.hpp>
#endif

#define BENCHMARK(NAME)     bench.context("benchmark_title", NAME);
#define SECTION(NAME)       bench.context("benchmark_name", NAME);
#define TEST_RPP(ACTION)    bench.context("source", "rpp").run(ACTION);
#ifdef RPP_BUILD_RXCPP
    #define TEST_RXCPP(ACTION)  bench.context("source", "rxcpp").run(ACTION);
#else
    #define TEST_RXCPP(ACTION)
#endif

char const* json() noexcept {
    return R"DELIM([
{{#result}}        {
            "title": "{{context(benchmark_title)}}",
            "name": "{{context(benchmark_name)}}",
            "source" : "{{context(source)}}",
            "median(elapsed)": {{median(elapsed)}},
            "medianAbsolutePercentError(elapsed)": {{medianAbsolutePercentError(elapsed)}}
        }{{^-last}},{{/-last}}
{{/result}}
])DELIM";
}


int main(int argc, char* argv[]) // NOLINT
{
    auto bench = ankerl::nanobench::Bench{}.output(nullptr).warmup(3);

    BENCHMARK("General")
    {
        SECTION("Subscribe empty callbacks to empty observable")
        {
            TEST_RPP([&]() 
            { 
                rpp::source::create<int>([&](auto&& observer) 
                { 
                    ankerl::nanobench::doNotOptimizeAway(observer); 
                }).subscribe([](int){}, [](const std::exception_ptr&){}, [](){});
            });

            TEST_RXCPP([&]() 
            {
                rxcpp::observable<>::create<int>([&](auto&& observer) 
                { 
                    ankerl::nanobench::doNotOptimizeAway(observer); 
                }).subscribe([](int){}, [](const std::exception_ptr&){}, [](){});
            });
        }
    };

    BENCHMARK("Sources")
    {
        SECTION("from array of 1 - create + subscribe + immediate")
        {
            std::array<int, 1> vals{123};
            TEST_RPP([&]() 
            { 
                rpp::source::from_iterable(vals).subscribe([](int v){ ankerl::nanobench::doNotOptimizeAway(v); }, [](const std::exception_ptr&){}, [](){});
            });

            TEST_RXCPP([&]() 
            {
                rxcpp::observable<>::iterate(vals).subscribe([](int v){ ankerl::nanobench::doNotOptimizeAway(v); }, [](const std::exception_ptr&){}, [](){});
            });
        }
    };

    if (argc > 1) {
        std::ofstream of{argv[1]};
        bench.render(json(), of);
        of.close();
    }
    
    bench.render(json(), std::cout);
}
