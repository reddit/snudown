#!/bin/env python

# dump all of our testcases into a directory as separate files, like AFL
# wants.

import os.path
import sys
import itertools

sys.path.append("..")
import test_snudown

cases = itertools.chain(test_snudown.cases.keys(), test_snudown.wiki_cases.keys())
for i, md in enumerate(cases):
    # skip huge testcases
    if len(md) > 2048:
        continue
    test_path = os.path.join('testing', 'testcases', 'test_default_%d.md' % i)
    with open(test_path, 'w') as f:
        f.write(md)
