import json
import sys
import os
from itertools import groupby

git_commit     = str(sys.argv[1])[:8]
commit_message = sys.argv[2].split("\n")[0] if len(sys.argv) > 2 else "Current PR"

new_data = {}

for file in os.listdir(os.fsencode("./artifacts")):
    platform = os.fsdecode(file)
    with open("./artifacts/"+platform+"/benchmarks_results.json", "r") as f:
        new_data[platform] = json.load(f)

with open("./gh-pages/v2/benchmark_results.json", "r") as f:
    res = json.load(f)

def print_metric(v):
    return f"{v:.2f} ns" if v else "-"

for platform, results in new_data.items():
    print(f"<details>\n<summary>\n\n## {platform}\n</summary>")
    for r in results:
        r["commit"] = git_commit
        r["commit_message"] = commit_message
        # convert to nanoseconds
        r["median(elapsed)"] *= 1e+9

    res.setdefault(platform, []).extend(results);

    for title, title_data in groupby(res[platform], lambda x:x["title"]):
        print(f"<details>\n<summary>\n\n### {title} \n</summary>\n\n")
        print("name | rxcpp | rpp | prev rpp | ratio")
        print("--- | --- | --- | --- | --- ")
        for name, name_data in groupby(title_data, lambda x: x["name"]):
            last_2_commits = [list(v) for _, v in groupby(name_data, lambda x: x["commit"])][-2:]

            prev_value = None
            if len(last_2_commits) == 2:
                prev_value = {k:list(v)[0] for k, v in groupby(last_2_commits[-2], lambda x: x["source"])}.get('rpp', {}).get('median(elapsed)')

            cur_vals = {k: list(v)[0] for k, v in groupby(last_2_commits[-1], lambda x: x["source"])}
            rpp_value = cur_vals.get('rpp', {}).get('median(elapsed)')
            rxcpp_value = cur_vals.get('rxcpp', {}).get('median(elapsed)')

            ratio = None
            if prev_value and rpp_value:
                ratio = rpp_value/prev_value

            print(f"{name} | {print_metric(rxcpp_value)} | {print_metric(rpp_value)}| {print_metric(prev_value)} | {print_metric(ratio)}")
        print("\n</details>")
    print(f"</details>")

with open("./gh-pages/v2/benchmark_results.json", "w") as f:
    json.dump(res, f, indent=4)
