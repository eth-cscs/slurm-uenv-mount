all: build run

login:
	docker login --username=${USER} ghcr.io --password-stdin
pull:
	docker pull ghcr.io/eth-cscs/slurm-container-20.11.9:latest

build:
	(cd .. &&\
	docker build \
		-f docker/Dockerfile \
		-t slurm-uenv-mount:latest .)

run:
	docker run -it --name slurm-uenv-mount --rm --privileged slurm-uenv-mount:latest /testuser-entrypoint.sh bash

run-detach:
	docker run --detach -it --name slurm-uenv-mount --rm --privileged slurm-uenv-mount:latest /testuser-entrypoint.sh bash

run-root:
	docker run -it --name slurm-uenv-mount --rm --privileged slurm-uenv-mount:latest bash

exec:
	docker exec -it -u testuser slurm-uenv-mount bash -l
