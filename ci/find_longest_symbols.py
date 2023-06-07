
def pretty_print(l):
    res = ""
    indent = 0
    for c in l:
        if c == "<":
            indent += 1
            res += c + "\n"+"\t"*indent
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

res = pretty_print(data[0])
print(res)