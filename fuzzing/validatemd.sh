#!/bin/bash
echo "** ${1}"
./build/snudown-validator < $1
