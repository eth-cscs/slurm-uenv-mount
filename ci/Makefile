
all: build run

build:
	docker build \
		-f ci/Dockerfile \
		-t slurm-uenv-mount:latest .

run:
	docker run -it slurm-uenv-mount:latest
