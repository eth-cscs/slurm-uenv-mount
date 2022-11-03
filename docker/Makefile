all: build run

build:
	(cd .. &&\
	docker build \
		-f docker/Dockerfile \
		-t slurm-uenv-mount:latest .)

run:
	docker run --name slurm-uenv-mount --rm --privileged -it slurm-uenv-mount:latest bash
