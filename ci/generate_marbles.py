#!/usr/bin/python3

import sys
import re
from rxmarbles import generator
import importlib
import os

theme = importlib.import_module('rxmarbles.theme.default')
gen_images_folder= os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'gen_docs', 'html'))
os.makedirs(gen_images_folder, exist_ok=True)

def generate_svg(name, text):
    parsed = generator.marble_diagrams.parseString(text)
    r = generator.get_objects(parsed[0][1:], theme)

    svg = generator.SvgDocument(r, theme, 75.0)
    with open(os.path.join(gen_images_folder, f"{name}.svg"), "w") as  f:
        f.write(svg.get_document())
        f.flush()
        os.fsync(f.fileno())

filename = sys.argv[1]
fileIn = open(filename, "r")

content = fileIn.readlines()

marble_name = None
marble_content = []
text_to_print = ""
for l in content:
    if marble_name is None:
        target="@marble "
        if target not in l:
            sys.stdout.write(l)
            continue
        i = l.index(target) +len(target)
        marble_name = l[i:].rstrip()
        start = l.find("*")
        text_to_print = f"{l[:start+1]}@image html {marble_name}.svg \r\n"
    else:
        if re.match(r"[\s]*\*.*", l):
            joined = '\n'.join(marble_content)
            generate_svg(marble_name, f"marble {marble_name} \n {joined}")
            marble_name= None
            marble_content = []
            sys.stdout.write(text_to_print)
            sys.stdout.write(l)
        else:
            marble_content.append(l)
