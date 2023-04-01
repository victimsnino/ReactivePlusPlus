#include "rpp/observers/fwd.hpp"
#include <nanobench.h>

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


int main() {
    auto bench = ankerl::nanobench::Bench{}.output(nullptr);

    BENCHMARK("Observables")
    {
        SECTION("subscribe on empty observer")
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

    bench.render(json(), std::cout);
}
