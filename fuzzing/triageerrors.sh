#!/bin/bash
find testing/afl_results/ -regextype posix-egrep -regex ".*/(crashes|hangs)/.*" | xargs -I '{}' ./validatemd.sh {}
