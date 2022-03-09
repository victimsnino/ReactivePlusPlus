import json

with open("./gh-pages/results.json", 'r') as f:
    results = json.load(f)   

for name, tests in results.items():
    print(f"# {name}")
    print("Test Name | Current, ns | Prev, ns | Ratio")
    print("--- | --- | ---")
    for test_name, data in tests.items():
        new_value = f"{data[-1]['val']}ns"
        old_value = f"{data[-2]['val']}ns" if len(data)> 1 else '-'
        ratio     = data[-1]['val']/data[-2]['val'] if len(data)> 1 else '-'
        print(f"{test_name} | { new_value } | { old_value } | {ratio}")
    
    print("")