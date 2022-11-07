
all: build run

login:
	docker login --username=${USER} ghcr.io --password-stdin
pull:
	docker pull ghcr.io/eth-cscs/slurm-container-20.11.9:latest

build:
	(cd .. &&\
	docker build \
		-f docker/Dockerfile.jpc \
		-t slurm-uenv-mount:latest .)

run:
	docker run -it slurm-uenv-mount:latest /testuser-entrypoint-jpc.sh
