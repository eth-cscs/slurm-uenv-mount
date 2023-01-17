#!/bin/bash


SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../"; pwd)"

docker run --name slurm-uenv-mount -w /home/testuser --volume="${SOURCE_DIR}":/slurm-uenv-mount:ro \
       --rm --privileged -it slurm-uenv-mount
