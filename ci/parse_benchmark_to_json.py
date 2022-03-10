import xml.etree.ElementTree as ET
import json
import sys
import pandas as pd

results = sys.argv[1]

root = ET.parse(results).getroot()

results = pd.DataFrame()

for test_case in root.findall("TestCase"):
    for result in test_case.findall("BenchmarkResults"):
        data = {"benchmark_name" : test_case.get("name"), "test_case" : result.get("name"), **{k:v for k, v in result.find('mean').items()}}
        results = pd.concat([results, pd.DataFrame(data=data, index=[0])]).reset_index(drop=True)

results.index.name = "id"
results.to_csv("parsed.csv")