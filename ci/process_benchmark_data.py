import os
import json
import sys

git_commit     = sys.argv[1] 
git_commit_message = sys.argv[2] if len(sys.argv) == 2 else "Current PR"

print(git_commit)

new_results = {}
for file in os.listdir(os.fsencode("./artifacts")):
     folder_with_results = os.fsdecode(file)
     with open(f'./artifacts/{folder_with_results}/parsed.json', 'r') as f:
        new_results[folder_with_results] = json.load(f)

old_results = {}
if os.path.exists("./gh-pages/results.json"):
    with open("./gh-pages/results.json", 'r') as f:
        old_results = json.load(f)   

for name, vals in new_results.items():
    old_results.setdefault(name, {})
    for test_name, val in vals.items():
        old_results[name].setdefault(test_name, [])
        old_results[name][test_name].append({"hash" : git_commit, "val" :val, "commit_message" : git_commit_message})
        
with open("./gh-pages/results.json", 'w') as f:
    json.dump(old_results, f)

print(old_results)

