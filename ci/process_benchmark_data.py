import plotly.offline as pyo
import plotly.graph_objects as go
from plotly.subplots import make_subplots

import os
import json
import sys
git_commit = sys.arv[1] 

new_results = {}
for file in os.listdir(os.fsencode("./artifacts")):
     folder_with_results = os.fsdecode(file)
     with open(f'./{folder_with_results}/results.json', 'r') as f:
        new_results[folder_with_results] = json.load(f)

with open("./gh-pages/results.json", 'r') as f:
    old_results = json.load(f)

for name, vals in new_results.items():
    old_results.setdefault(name, {})
    for test_name, val in vals.items():
        old_results[name].setdefault(test_name, [])
        old_results[name][test_name].append([git_commit, val])
        
with open("gh-pages/results.json", 'w') as f:
    json.dump(old_results, f)

print(old_results)

# filename = "result.html"
# dashboard = open(filename, 'w')
# dashboard.write("<html><head></head><body>" + "\n")
# add_js = True

# def dump_plot(fig):
#     global add_js
#     global dashboard

#     dashboard.write(pyo.plot(fig, include_plotlyjs=add_js, output_type='div'))
#     add_js = False


# fig = make_subplots(rows=3, cols=1, shared_xaxes=True, vertical_spacing=0.01)

# fig.add_trace(go.Scatter(x=[1,2,3], y=[10, 11, 12], name="t"), row=3, col=1,)
# fig.add_trace(go.Scatter(x=[1,2,5], y=[100, 110, 120]), row=2, col=1)
# fig.add_trace(go.Scatter(x=[1,2,4], y=[1000, 1100, 1200]), row=1, col=1)
# fig.update_xaxes(tickvals=[1,2,3,4,5], ticktext=["1f", "2f", "3f", "4f", "5f"])

# fig.update_layout(title_text="Stacked Subplots with Shared X-Axes", title_x=0.5)
# fig.update_xaxes(title_text='Commit')
# fig.update_yaxes(title_text='Duration')

# dump_plot(fig)
# dashboard.write("<h1>Some text</h1> ")



# dashboard.write("</body></html>" + "\n")
# dashboard.close()

