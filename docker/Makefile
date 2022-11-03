all: build run

build:
	(cd .. &&\
	docker build \
		-f docker/Dockerfile \
		-t slurm-uenv-mount:latest .)

run:
	docker run -it slurm-uenv-mount:latest bash
