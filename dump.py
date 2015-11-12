#!/usr/bin/env python
__doc__ = """Simple script to dump data contents of test.xml"""

import sys

data = open('test.xml').read()

arguments = sys.argv[:]
if len(arguments) == 1:
    start = 0
    stop = 0
elif len(arguments) == 2:
    start = int(arguments[1])
    stop = 0
elif len(arguments) == 3:
    start = int(arguments[1])
    stop = int(arguments[2])

# 32 bit Unicode symbols
UNICODE_SIZE = 4

if stop == 0:
    stop = len(data)/UNICODE_SIZE

start = start * UNICODE_SIZE
stop = stop * UNICODE_SIZE

for index in range(start, stop, UNICODE_SIZE):
    sys.stdout.write(data[index])
    #if not index % 16: print
print
