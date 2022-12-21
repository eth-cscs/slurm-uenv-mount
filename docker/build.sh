#!/usr/bin/bash

set -x

TAG=slurm-uenv-mount

SOURCE_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"

(
    cd ../ || exit 1
    BUILDKIT_PROGRESS=plain docker build . -f "${SOURCE_DIR}/Dockerfile" --progress=plain -t $TAG
)
