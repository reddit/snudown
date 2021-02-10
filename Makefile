PACKAGE := snudown
VERSION := 1.6.0
DOCKERFILE := Dockerfile.wheel

default: build run

build:
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
