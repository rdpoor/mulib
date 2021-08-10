'''
Recursively list directories, sub-directories and files.
'''
import os
import sys

def list_files(startpath):
    for root, dirs, files in os.walk(startpath):
        dirs.sort()
        level = root.replace(startpath, '').count(os.sep)
        indent = '|   ' * (level)
        print('{}{}/'.format(indent, os.path.basename(root)))
        subindent = '|   ' * (level + 1)
        for f in files:
            print('{}{}'.format(subindent, f))

if len(sys.argv) < 2:
    list_files('.')
else:
    list_files(sys.argv[1])
