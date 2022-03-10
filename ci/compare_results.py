import json
import pandas as pd

results = pd.read_csv("./gh-pages/results.csv", index_col="id") 

for platform, data in results.groupby("platform"):
    indexes = data["commit"].unique()[-2:]
    test_data =[v[1] for v in data[data["commit"].isin(indexes)].groupby("commit", sort=False, as_index=False)]
    cur_data = test_data[-1]
    prev_data = test_data[-2] if len(test_data) >= 2 else pd.DataFrame(columns=cur_data.columns)

    print(f"# {platform}")

    for name, bench_data in cur_data.groupby("benchmark_name"):
        print(f"## {name}")
        print("<details>")
        print("<summary>Table</summary>")
        print("")
        print("Test Name | Current, ns | Prev, ns | Ratio")
        print("--- | --- | --- | ---")
        for _, r in bench_data.iterrows():
            old_data = prev_data[(prev_data['benchmark_name']==name) & (prev_data['test_case'] == r['test_case'])]
            new_value = f"{r['value']:.2f}ns"
            old_value = old_data['value'].values[0] if len(old_data['value'].values) >= 1 else None
            old_value_str = f"{old_value:.2f}ns" if old_value is not None else "."
            ratio     = f"{r['value']/old_value:.2f}" if old_value is not None else '.'
            print(f"{r['test_case']} | { new_value } | { old_value } | {ratio}")
        print("")
        print("</details>")
        print("")
