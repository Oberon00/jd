#!/usr/bin/env python
# -*- coding: utf-8 -*-

from zipfile import ZipFile, ZIP_DEFLATED
from os import path
import os
import sys

def write_to_zip_file(zip_file, directory):
    for dirpath, subdirs, files in os.walk(directory):
        for objectt in (subdirs, files):
            for opath in objectt:
                opath = path.join(dirpath, opath)
                zip_file.write(opath)

def main():
    if len(sys.argv) < 3:
        sys.exit("Usage: mkzip.py ZIP {INPUTS}")
    with ZipFile(sys.argv[1], 'w', ZIP_DEFLATED) as zip_file:
        for input in sys.argv[2:]:
            if path.isdir(input):
                write_to_zip_file(zip_file, input)
            else:
                zip_file.write(input)

if __name__ == '__main__':
    main()