#include "rpp/observers/fwd.hpp"
#include <nanobench.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string_view>

#include <rpp/rpp.hpp>
#ifdef RPP_BUILD_RXCPP
    #include <rxcpp/rx.hpp>
#endif

#define BENCHMARK(NAME)     bench.title(NAME);
#define SECTION(NAME)       bench.name(NAME);
#define TEST_RPP(ACTION)    bench.context("source", "rpp").run(ACTION);
#ifdef RPP_BUILD_RXCPP
    #define TEST_RXCPP(ACTION)  bench.context("source", "rxcpp").run(ACTION);
#else
    #define TEST_RXCPP(ACTION)
#endif

char const* json() noexcept {
    return R"DELIM([
{{#result}}        {
            "title": "{{title}}",
            "name": "{{name}}",
            "source" : "{{context(source)}}",
            "median(elapsed)": {{median(elapsed)}},
            "medianAbsolutePercentError(elapsed)": {{medianAbsolutePercentError(elapsed)}}
        }{{^-last}},{{/-last}}
{{/result}}
])DELIM";
}


int main(int argc, char* argv[]) 
{
    auto bench = ankerl::nanobench::Bench{}.output(nullptr);

    BENCHMARK("General")
    {
        SECTION("Subscribe empty callbacks to empty observable")
        {
            TEST_RPP([&]() 
            { 
                rpp::make_lambda_observable<int>([&](auto&& observer) 
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

    if (argc > 1) {
        std::ofstream of{argv[1]};
        bench.render(json(), of);
        of.close();
    }
    
    bench.render(json(), std::cout);
}
