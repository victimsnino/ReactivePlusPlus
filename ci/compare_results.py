import json

with open("./gh-pages/results.json", 'r') as f:
    results = json.load(f)   

for name, tests in results.items():
    print(f"# {name}")
    print("Test Name | Prev | Current")
    print("--- | --- | ---")
    for test_name, data in tests.items():
        print(f"{test_name} | { data[-2]['val'] } | { data[-1]['val'] }")
    
    print("")