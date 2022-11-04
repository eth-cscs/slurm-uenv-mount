all: build run

build:
	(cd .. &&\
	docker build \
		-f docker/Dockerfile \
		-t uenv-mount-rootless:latest .)

run:
	docker run --name uenv-mount-rootless --rm --privileged -it uenv-mount-rootless bash
