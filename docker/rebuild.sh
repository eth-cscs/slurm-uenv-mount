#!/usr/bin/bash

docker exec -w /slurm-uenv-mount slurm-uenv-mount sh -c 'meson setup --prefix=/usr /tmp/build; ninja install -C /tmp/build; echo "required /usr/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf'
