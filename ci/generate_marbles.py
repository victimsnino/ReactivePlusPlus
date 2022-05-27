from rxmarbles import generator
import importlib
import os
import fnmatch
import re

theme = importlib.import_module('rxmarbles.theme.default')
gen_images_folder= os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'gen_images'))
os.makedirs(gen_images_folder, exist_ok=True)

def generate_svg(name, text):
    parsed = generator.marble_diagrams.parseString(text)
    r = generator.get_objects(parsed[0][1:], theme)

    svg = generator.SvgDocument(r, theme, 75.0)
    with open(os.path.join(gen_images_folder, f"{name}.svg"), "w") as  f:
        f.write(svg.get_document())

rpp_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'src', 'rpp'))


for root, dirnames, filenames in os.walk(rpp_dir):
    for file in fnmatch.filter(filenames, '*.hpp'):
        with open(os.path.join(root, file), 'r') as f:
            content = f.read()
        r = re.findall(r"marble\{([a-zA-Z_]*),\s*(\{[\s\S]*\})\s*}\s*\*", content)
        for marble in r:
            generate_svg(marble[0], f"marble {marble[0]} \n {marble[1]}")



