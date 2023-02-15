#!/usr/bin/bash

docker compose exec -w /slurm-uenv-mount slurm \
    sh -c 'mkdir -p /tmp/build \
        && cd /tmp/build \
        && make -f /slurm-uenv-mount/Makefile all install \
        && echo "required /usr/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf'
