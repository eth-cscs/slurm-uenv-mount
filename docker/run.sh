#!/bin/bash


SOURCE_DIR=$(realpath $(dirname "$(realpath "${BASH_SOURCE[0]}")")/../)

docker run --name slurm-uenv-mount --volume="${SOURCE_DIR}":/slurm-uenv-mount:ro \
       --rm --privileged -it slurm-uenv-mount "$@"
