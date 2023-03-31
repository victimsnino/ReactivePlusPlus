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
    return R"DELIM({
    "results": [
{{#result}}        {
            "title": "{{title}}",
            "name": "{{name}}",
            "source" : "{{context(source)}}",
            "maxEpochTime": {{maxEpochTime}},
            "minEpochTime": {{minEpochTime}},
            "relative": {{relative}},
            "median(elapsed)": {{median(elapsed)}},
            "medianAbsolutePercentError(elapsed)": {{medianAbsolutePercentError(elapsed)}},
            "totalTime": {{sumProduct(iterations, elapsed)}}         ]
        }{{^-last}},{{/-last}}
{{/result}}    ]
})DELIM";
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


/*<html>

<head>
    <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
</head>

<body>
    <div id="graphs"></div>
    <script>
        fetch('temp_temp.json')
            .then(res => res.json())
            .then(json => {
                var title = 'Addition';

                var grouped = json.results
                    .reduce((acc, v) => {
                        group = v["foo"]
                        acc[group] = (acc[group] || []);
                        acc[group].push(v);
                        return acc;
                    }, {});

                    for (const [key, value] of Object.entries(grouped)) {
                        var layout = {
                            <!-- title: { text: key }, -->
                            showlegend: true,
                            hovermode: "x unified",
                            yaxis: { title: 'time per unit', rangemode: 'tozero', autorange: true }
                        };

                        var elemDiv = document.createElement('div');
                        var details = document.createElement('details');
                        var summary = document.createElement('summary');
                        summary.innerHTML += key;

                        Plotly.newPlot(details, value.map(v => Object.assign({ name: v["title"], x: [5, 15], y: [v["median(elapsed)"], v["totalTime"]], type: 'scatter' })), layout, {responsive: true});

                        details.appendChild(summary);
                        elemDiv.appendChild(details);
                        document.getElementById("graphs").appendChild(elemDiv);
                    }
            });
    </script>
</body>

</html>*/