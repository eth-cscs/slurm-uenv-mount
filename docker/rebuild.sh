#!/usr/bin/bash

docker exec -w /slurm-uenv-mount slurm-uenv-mount \
    sh -c 'mkdir -p /tmp/build \
        && cd /tmp/build \
        && make -f /slurm-uenv-mount/Makefile all install \
        && echo "required /usr/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf'
