import json
import os

with open("files.json", "r", encoding="utf-8") as json_file:
    data = json.load(json_file)

for file_info in data:
    os.makedirs(os.path.dirname(file_info["file_path"]), exist_ok=True)
    with open(file_info["file_path"], "w", encoding="utf-8") as f:
        f.write(file_info["file_content"])
