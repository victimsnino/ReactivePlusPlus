import sys
import re
from rxmarbles import generator
import importlib
import os

theme = importlib.import_module('rxmarbles.theme.default')
gen_images_folder= os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'gen_images'))
os.makedirs(gen_images_folder, exist_ok=True)

def generate_svg(name, text):
    parsed = generator.marble_diagrams.parseString(text)
    r = generator.get_objects(parsed[0][1:], theme)

    svg = generator.SvgDocument(r, theme, 75.0)
    with open(os.path.join(gen_images_folder, f"{name}.svg"), "w") as  f:
        f.write(svg.get_document())

filename = sys.argv[1]
fileIn = open(filename, "r")

content = fileIn.readlines()

marble_name = None
marble_content = []
for l in content:
    if marble_name is None:
        target="\\marble "
        if target not in l:
            sys.stdout.write(l)
            continue
        i = l.index(target) +len(target)
        marble_name = l[i:].rstrip()
        start = l.find("*")
        sys.stdout.write(l.replace("\\marble", "\\par Marble").replace(marble_name, f"\n{l[:start+1]} \\image html {marble_name}.svg"))
    else:
        if re.match(r"[\s]*\*.*", l):
            joined = '\n'.join(marble_content)
            generate_svg(marble_name, f"marble {marble_name} \n {joined}")
            marble_name= None
            marble_content = []
            sys.stdout.write(l)
        else:
            marble_content.append(l)



# import fnmatch
# import re
# import shutil

# rpp_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'src', 'rpp'))


# for root, dirnames, filenames in os.walk(rpp_dir):
#     for file in fnmatch.filter(filenames, '*.hpp'):
#         with open(os.path.join(root, file), 'r') as f:
#             content = f.readlines()
        
#         marble_name = None
#         marble_content = []
#         for l in content:
#             if marble_name is None:
#                 target="\marble{"
#                 if target not in l:
#                     continue
#                 i = l.index(target) +len(target)
#                 marble_name = l[i:l.index(',', i)]
#             else:
#                 if re.match(r"[\s]*\*.*", l):
#                     joined = '\n'.join(marble_content)
#                     generate_svg(marble_name, f"marble {marble_name} \n {joined}")
#                     marble_name= None
#                     marble_content = []
#                 else:
#                     marble_content.append(l)



