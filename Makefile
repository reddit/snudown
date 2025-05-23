PACKAGE := snudown
DRONE_TAG ?= 0.0.0
SNUDOWN_VERSION := $(DRONE_TAG)
export SNUDOWN_VERSION
DOCKERFILE := Dockerfile.wheel

.PHONY: default build build-linux build-macos clean

default: clean build-linux build-macos

build: build-linux build-macos

build-linux:
	bash scripts/build-linux.sh 2.7
	bash scripts/build-linux.sh 3.9
	bash scripts/build-linux.sh 3.10
	bash scripts/build-linux.sh 3.11
	bash scripts/build-linux.sh 3.12

build-macos:
	bash scripts/build-macos.sh 3.9
	bash scripts/build-macos.sh 3.10
	bash scripts/build-macos.sh 3.11
	bash scripts/build-macos.sh 3.12

clean:
	rm -rf build
	rm -rf dist
