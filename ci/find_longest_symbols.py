
def indent(l):
    res = ""
    indent = 0
    for c in l:
        if c == "<":
            res += "\n"+"\t"*indent+ c
            indent += 1
            res += "\n"+"\t"*indent
        elif c == ">":
            indent -= 1
            res += "\n"+"\t"*indent + c
        elif c == ",":
            res += "\n"+"\t"*indent + c
        else:
            res += c
    return res

with open("../build/bin/dump.txt") as f:
    data = f.readlines()

data = [l for l in data if "rpp" in l]
data = [l for l in data if "simulate_" not in l]

print(f"Total lines: {len(data)}")

data = sorted(data, key=len, reverse=True)

print(f"Longest line: {len(data[0])}")

for i in range(min(len(data), 10)):
    print(f"Length: {len(data[i])}")
    print(indent(data[i]))
