#include <nanobench.h>

#include <cmath>
#include <iostream>

namespace {

template <typename T>
void fma() {
    T x(1);
    T y(2);
    T z(3);
    z = std::fma(x, y, z);
    ankerl::nanobench::doNotOptimizeAway(z);
}

template <typename T>
void plus_eq() {
    T x(1);
    T y(2);
    T z(3);
    z += x * y;
    ankerl::nanobench::doNotOptimizeAway(z);
}

// char const* csv() {
//     return R"DELIM("title";"name";"scalar";"foo";"elapsed";"total"
// {{#result}}"{{title}}";"{{name}}";"{{context(scalar)}}";"{{context(foo)}}";{{median(elapsed)}};{{sumProduct(iterations, elapsed)}}
// {{/result}})DELIM";
// }

char const* json() noexcept {
    return R"DELIM({
    "results": [
{{#result}}        {
            "title": "{{title}}",
            "name": "{{name}}",
            "foo" : "{{context(foo)}",
            "unit": "{{unit}}",
            "batch": {{batch}},
            "complexityN": {{complexityN}},
            "epochs": {{epochs}},
            "clockResolution": {{clockResolution}},
            "clockResolutionMultiple": {{clockResolutionMultiple}},
            "maxEpochTime": {{maxEpochTime}},
            "minEpochTime": {{minEpochTime}},
            "minEpochIterations": {{minEpochIterations}},
            "epochIterations": {{epochIterations}},
            "warmup": {{warmup}},
            "relative": {{relative}},
            "median(elapsed)": {{median(elapsed)}},
            "medianAbsolutePercentError(elapsed)": {{medianAbsolutePercentError(elapsed)}},
            "median(instructions)": {{median(instructions)}},
            "medianAbsolutePercentError(instructions)": {{medianAbsolutePercentError(instructions)}},
            "median(cpucycles)": {{median(cpucycles)}},
            "median(contextswitches)": {{median(contextswitches)}},
            "median(pagefaults)": {{median(pagefaults)}},
            "median(branchinstructions)": {{median(branchinstructions)}},
            "median(branchmisses)": {{median(branchmisses)}},
            "totalTime": {{sumProduct(iterations, elapsed)}}
        }{{^-last}},{{/-last}}
{{/result}}    ]
})DELIM";
}

} // namespace

// NOLINTNEXTLINE
int main() {
    ankerl::nanobench::Bench bench;
    bench.title("Addition").output(nullptr);
    bench.context("scalar", "f32")
        .context("foo", "bar")
        .run("+=", plus_eq<float>)
        .run("fma", fma<float>);
    bench.context("scalar", "f64")
        .context("foo", "baz")
        .run("+=", plus_eq<double>)
        .run("fma", fma<double>);

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