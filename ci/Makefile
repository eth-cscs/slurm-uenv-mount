TAG?=slurm-uenv-mount:latest
DOCKER_CONTAINER?=ghcr.io/eth-cscs/slurm-container-20.11.9:latest

all: build run

build:
	(cd .. &&\
	docker build \
		-f ci/Dockerfile \
		--build-arg DOCKER_CONTAINER=${DOCKER_CONTAINER} \
		-t ${TAG} .)

run:
	docker run -it ${TAG}
