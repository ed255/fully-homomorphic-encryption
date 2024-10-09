#!/usr/bin/env python3

import sys
from os import listdir
from os.path import isfile, join

def scan(src_file):
    lines = []
    with open(src_file) as file:
        lines = [line.rstrip() for line in file]
    level_size = []
    size = 0
    in_level = False
    for line in lines:
        if line.startswith('static LEVEL_'):
            in_level = True
            continue
        if in_level:
            size += 1
            if line == '':
                in_level = False
                level_size.append(size)
                size = 0
    return level_size

src_file = sys.argv[1]

print(f'{src_file}')
level_size = scan(src_file)
print(f'- gates: {sum([size for size in level_size])}')
print(f'- levels: {len(level_size)}')
print(f'- levels_size: {level_size}')
print()
