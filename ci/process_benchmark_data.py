import os
import json
import sys

git_commit     = sys.argv[1] 
git_commit_url = sys.argv[2] 

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
        old_results[name][test_name].append({"hash" : git_commit, "url" : git_commit_url, "val" :val})
        
with open("gh-pages/results.json", 'w') as f:
    json.dump(old_results, f)

print(old_results)

