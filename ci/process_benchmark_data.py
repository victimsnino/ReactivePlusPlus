import os
import json
import sys
import pandas as pd
from parse_benchmark_to_json import parse

git_commit     = str(sys.argv[1]) 
commit_message = sys.argv[2].split("\n")[0] if len(sys.argv) > 2 else "Current PR"

new_results = pd.DataFrame()
for file in os.listdir(os.fsencode("./artifacts")):
    folder_with_results = os.fsdecode(file)
    new_data = parse(f'./artifacts/{folder_with_results}/benchmark_result.txt')
    new_data["platform"] = [folder_with_results]*len(new_data)
    new_data["commit"] = [f"{commit_message[:20]}{'...' if len(commit_message)> 20 else ''} ({git_commit})"]*len(new_data)
    new_results = pd.concat([new_results, new_data]).reset_index(drop=True)

old_results = pd.DataFrame()
if os.path.exists("./gh-pages/results.csv"):
    old_results = pd.read_csv("./gh-pages/results.csv", index_col="id")

results = pd.concat([old_results, new_results]).reset_index(drop=True)
results.index.name = "id"
results.to_csv("./gh-pages/results.csv")
