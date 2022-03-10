import plotly.offline as pyo
import plotly.express as px
from plotly.subplots import make_subplots
import pandas as pd
import plotly.graph_objects as go


dashboard = open("./gh-pages/benchmark.html", 'w')
dashboard.write("<html><head></head><body>" + "\n")
add_js = True

def dump_plot(fig):
    global add_js
    global dashboard

    dashboard.write(pyo.plot(fig, include_plotlyjs=add_js, output_type='div'))
    add_js = False


results = pd.read_csv("./gh-pages/results.csv", index_col="id")

# duplicate last row to fix issue with splines
results = pd.concat([results, results[results['commit'] == results["commit"].unique()[-1]]]).reset_index(drop=True)

for platform, data in results.groupby("platform", sort=False, as_index=False):
    dashboard.write(f"<h2>{platform} </h2>")
    for i, (name, bench_data) in enumerate(data.groupby("benchmark_name", sort=False, as_index=False)):
        hover_data = {"commit": False, "min" : bench_data["lowerBound"], "max": bench_data["upperBound"]}
        labels={
                     "value": "ns/iter",
                     "commit": "Commit",
                     "test_case" : "Benchmark"
                 }
        fig = px.line(bench_data, x="commit", y="value", color="test_case", line_shape='spline', markers=True, hover_data=hover_data, title=name, height=500, labels=labels)
        copy_data = fig["data"]
        for v in copy_data:
            d = bench_data[bench_data['test_case']==v['legendgroup']]
            fig.add_trace(go.Scatter(x=pd.concat([d['commit'], d['commit'][::-1]]),
                                     y=pd.concat([d['lowerBound'], d['upperBound'][::-1]]),
                                     fill='toself',
                                     showlegend=False,
                                     name = v['legendgroup'],
                                     line_color=v['line']['color'],
                                     hoverinfo='skip',
                                     mode="lines",
                                     opacity=0.3,
                                     line_shape='spline'
                                ))
        fig.update_layout(hovermode="x", title_x=0.5)
        fig.update_xaxes(tickangle=-35)
        dump_plot(fig)


dashboard.write("</body></html>" + "\n")
dashboard.close()
