import json

with open("./gh-pages/results.json", 'r') as f:
    results = json.load(f)   

for name, tests in results.items():
    print(f"# {name}")
    print("Test Name | Current | Prev")
    print("--- | --- | ---")
    for test_name, data in tests.items():
        print(f"{test_name} | { data[-1]['val'] } | { data[-2]['val'] if len(data)> 1 else '-' }")
    
    print("")