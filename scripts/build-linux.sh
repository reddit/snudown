#!/usr/bin/env bash

set -ex

VERSION=${1}

echo build ${VERSION}
docker build \
  --platform linux/amd64 \
  --target py${VERSION} \
  --build-arg SNUDOWN_VERSION \
  -t test:py${VERSION} \
  -f Dockerfile.wheel \
  .

mkdir -p dist
# linux
docker run \
  --platform linux/amd64 \
  --rm \
  -e SNUDOWN_VERSION \
  -v `pwd`/dist:/tmp/dist \
  -it \
  test:py${VERSION}
