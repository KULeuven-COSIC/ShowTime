#!/usr/bin/env python3

import re
import statistics
import sys

file = open(sys.argv[1])

cached = []
flushed = []

for line in file.readlines():
    cm = re.match(
        r"Cached, should see lots of 0xFFs :\s*\d* \/ 10000 \(([\d.]+)%", line)
    if cm:
        cached.append(float(cm.group(1)))
    em = re.match(
        r"Not cached, should see few 0xFFs :\s*\d* \/ 10000 \(([\d.]+)%", line)
    if em:
        flushed.append(float(em.group(1)))

print("Cached median rate: {0}, minimum: {1}".format(
    statistics.median(cached), min(cached)))
print("Non-cached median rate: {0}, maximum: {1}".format(
    statistics.median(flushed), max(flushed)))
