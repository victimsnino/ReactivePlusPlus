import plotly.offline as pyo
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import json



dashboard = open("./gh-pages/benchmark.html", 'w')
dashboard.write("<html><head></head><body>" + "\n")
add_js = True

def dump_plot(fig):
    global add_js
    global dashboard

    dashboard.write(pyo.plot(fig, include_plotlyjs=add_js, output_type='div'))
    add_js = False


with open("./gh-pages/results.json", 'r') as f:
    results = json.load(f)  

for name, tests in results.items():
    fig = make_subplots(rows=len(tests), cols=1, shared_xaxes=True, subplot_titles=list(tests.keys()))

    for i, (test_name, data) in enumerate(tests.items()):
        x = [v["hash"] for v in data]
        y = [v["val"] for v in data]
        commit_message = [v["commit_message"] for v in data]
        fig.add_trace(go.Scatter(x=x, y=y, customdata=commit_message, name=test_name, hovertemplate='<br>value:%{y}<br>commit:%{customdata}'), row=i+1, col=1)

    fig.update_layout(title_text=name, title_x=0.5, showlegend=False, height=180*len(tests))
    fig.update_xaxes(title_text='Commit')
    fig.update_yaxes(title_text='ns/iter')
    dump_plot(fig)

dashboard.write("</body></html>" + "\n")
dashboard.close()
