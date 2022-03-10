import xml.etree.ElementTree as ET
import json
import sys

results = sys.argv[1]

root = ET.parse(results).getroot()

results = {}
for result in root.findall("TestCase/BenchmarkResults"):
    results[result.get('name')] = result.find('mean').get('value')

with open('parsed.json', 'w') as r:
    json.dump(results, r, indent=4)
