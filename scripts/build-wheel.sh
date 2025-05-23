#!/usr/bin/env bash
export WHEEL_OUTPUT_DIR=${WHEEL_OUTPUT_DIR:-dist}
python -m pip install -U pip wheel setuptools
python setup.py install
python test_snudown.py
python -m pip wheel --wheel-dir=${WHEEL_OUTPUT_DIR} .
ls -la dist

