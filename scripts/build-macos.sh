#!/usr/bin/env bash

set -ex

#export CC=/opt/homebrew/bin/gcc-13

VERSION=${1}

if [ ! -d "venv-${VERSION}" ]; then
  python${VERSION} -m venv venv-${VERSION}
fi

source venv-${VERSION}/bin/activate
rm -rf build
python -m pip install -U pip wheel setuptools
python setup.py install
python test_snudown.py
python -m pip wheel --wheel-dir=dist .
python test_snudown.py
