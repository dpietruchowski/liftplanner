import glob
import json
import os

pattern = "*src/repositories/*.h"
output_file = "output_files.json"

files_data = []

for file_path in glob.glob(pattern):
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()
    files_data.append({
        "file_path": file_path,
        "content": content
    })

with open(output_file, "w", encoding="utf-8") as f:
    json.dump(files_data, f, ensure_ascii=False, indent=4)

print(f"File '{output_file}' has been created with all files matching '{pattern}'.")
