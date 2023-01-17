#!/usr/bin/bash

set -x

TAG=slurm-uenv-mount
CONTAINER=$1

if [ ! -z "$CONTAINER" ]; then
    BUILD_ARGS+="--build-arg DOCKER_CONTAINER=${CONTAINER}"
fi

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../"; pwd)"

(
    cd ${SOURCE_DIR} || exit 1
    BUILDKIT_PROGRESS=plain docker build . -f "docker/Dockerfile" ${BUILD_ARGS} --progress=plain -t $TAG
)
