#!/usr/bin/bash

docker compose exec -w /slurm-uenv-mount slurm \
    sh -c "meson install -C /uenv-build"
