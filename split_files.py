#!/usr/bin/env python3
import re
from pathlib import Path
import os

file_path = os.getenv('file')
if not file_path:
    raise RuntimeError('Must pass a file')
file_path = Path(file_path)

file_pattern = re.compile(r'file: (?P<name>.*?) \{\n(?P<content>.*?)\}', re.DOTALL)

with file_path.open() as file:
    file_string = file.read()
    for match in file_pattern.finditer(file_string):
        sub_file_path = match.group('name')
        sub_file_path = sub_file_path.replace('$parent', file_path.stem)
        sub_file_path = file_path.parent / sub_file_path
        print(sub_file_path)
        with sub_file_path.open('w') as sub_file:
            sub_file.write(match.group('content'))
