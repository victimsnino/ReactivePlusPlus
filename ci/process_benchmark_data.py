import os
import json
import sys
import pandas as pd
from parse_benchmark_to_json import parse

git_commit     = str(sys.argv[1])[:8]
commit_message = sys.argv[2].split("\n")[0] if len(sys.argv) > 2 else "Current PR"

new_results = pd.DataFrame()
for file in os.listdir(os.fsencode("./artifacts")):
    folder_with_results = os.fsdecode(file)
    for source in ['rpp', 'rxcpp', 'rpp_type_erased_value']:
        file_name = f'./artifacts/{folder_with_results}/{source}_benchmark_result.txt'
        if not os.path.exists(file_name):
            continue
        new_data = parse(file_name)
        new_data["platform"] = [folder_with_results]*len(new_data)
        new_data["commit"] = [f"{commit_message[:20]}{'...' if len(commit_message)> 20 else ''} ({git_commit})"]*len(new_data)
        new_data["source"] = [source]*len(new_data)
        new_results = pd.concat([new_results, new_data]).reset_index(drop=True)

old_results = pd.DataFrame()
if os.path.exists("./gh-pages/results.csv"):
    old_results = pd.read_csv("./gh-pages/results.csv", index_col="id")
    if 'source' not in old_results:
        old_results['source'] = ['rpp']*len(old_results)
    if 'is_rxcpp' in old_results:
        old_results = old_results.drop(columns=['is_rxcpp'])
    if 'line_dash' in old_results:
        old_results = old_results.drop(columns=['line_dash'])

results = pd.concat([old_results, new_results]).reset_index(drop=True)
results.index.name = "id"
results.to_csv("./gh-pages/results.csv")
