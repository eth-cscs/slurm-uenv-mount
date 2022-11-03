
all: build run

build:
	docker build \
		-f ci/Dockerfile \
		--build-arg DOCKER_CONTAINER=ghcr.io/eth-cscs/slurm-container-20.11.9:latest \
		-t slurm-uenv-mount:latest .

run:
	docker run -it slurm-uenv-mount:latest
