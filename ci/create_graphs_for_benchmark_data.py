import plotly.offline as pyo
import plotly.express as px
from plotly.subplots import make_subplots
import pandas as pd
import plotly.graph_objects as go

def rindex(lst, value):
    return len(lst) - lst[::-1].index(value) - 1
    
dashboard = open("./gh-pages/benchmark.html", 'w')
dashboard.write("<html><head></head><body>" + "\n")
dashboard.write("<p> TIP: Each graph can be zoomed in via selection of interested region! Double-click to return to original zoom mode </p>")
add_js = True

def dump_plot(fig, name):
    global add_js
    global dashboard

    dashboard.write(f"<details> <summary><b>{name}</b></summary>")
    dashboard.write(pyo.plot(fig, include_plotlyjs=add_js, output_type='div'))
    dashboard.write("</details><br>")

    add_js = False


results = pd.read_csv("./gh-pages/results.csv", index_col="id")
all_commits = list(results["commit"].unique())
take_last=20
# duplicate last row to fix issue with splines
results = pd.concat([results, results[results['commit'] == results["commit"].unique()[-1]]]).reset_index(drop=True)

colormap = px.colors.qualitative.Plotly
for platform, data in results.groupby("platform", sort=False, as_index=False):
    dashboard.write(f"<h2>{platform} </h2>")
    for name, bench_data in data.groupby("benchmark_name", sort=False, as_index=False):
        fig = go.Figure()
        for i, (test_case, test_cases_data) in enumerate(bench_data.groupby("test_case", sort=False, as_index=False)):
            for source, source_data in test_cases_data.groupby("source", sort=False, as_index=False):
                commit_indexes=[all_commits.index(c) for c in source_data["commit"]]
                fig.add_trace(go.Scatter(x=commit_indexes,
                                         y=source_data["value"],
                                         line_shape='spline',
                                         mode='lines+markers',
                                         marker_color=colormap[i],
                                         line_color=colormap[i],
                                         line_dash='solid' if source == 'rpp' else 'dot',
                                         name=f'{test_case}, {source}'))
                if source == 'rpp':
                    fig.add_trace(go.Scatter(
                        x=commit_indexes + commit_indexes[::-1],
                        y=pd.concat([source_data['lowerBound'],
                                    source_data['upperBound'][::-1]]),
                        fill='toself',
                        fillcolor=colormap[i],
                        line_color=colormap[i],
                        name=f'{test_case}, {source}',
                        showlegend=False,
                        mode="lines",
                        opacity=0.3,
                        line_shape='spline',
                        hoverinfo='skip'
                    ))

        min_val = bench_data.groupby("commit", sort=False)["value"].agg(["min"])[-take_last:].min().values[0]
        max_val = bench_data.groupby("commit", sort=False)["value"].agg(["max"])[-take_last:].max().values[0]
        diff = (max_val - min_val) * 0.05
        min_val -= diff
        max_val += diff
        fig.update_layout(
            hovermode="x unified",
            title_x=0.5,
            title=name,
            xaxis_title="Commit",
            yaxis_title="ns/iter",
            legend_title="Legend Title",
            xaxis=dict(
                tickmode='array',
                tickvals=list(range(0, len(all_commits))),
                ticktext=all_commits,
                tickangle=-35,
                rangeslider=dict(visible=True)
            ),
            yaxis=dict(
                # autorange=True,
                fixedrange=False
            ))

        fig['layout']['xaxis'].update(range=[len(all_commits)-take_last, len(all_commits)])
        fig['layout']['yaxis'].update(range=[min_val, max_val])
        
        dump_plot(fig, name)


dashboard.write("</body></html>" + "\n")
dashboard.close()
