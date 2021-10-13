PACKAGE := snudown
VERSION := $(shell grep -oP 'SNUDOWN_VERSION "\K\d.\d.\d' snudown.c)
DOCKERFILE := Dockerfile.wheel

default: clean build run

build:
	echo $(VERSION)
	docker build \
    -t $(PACKAGE):$(VERSION) \
    -f $(DOCKERFILE) \
		.

run:
	mkdir -p dist
	docker run \
		--rm \
		-v `pwd`/dist:/tmp/dist \
		-it \
    $(PACKAGE):$(VERSION)

clean:
	rm -rf build
	rm -rf dist
