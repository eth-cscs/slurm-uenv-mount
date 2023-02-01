#!/usr/bin/bash

TAG=slurm-uenv-mount
CONTAINER=$1

if [ ! -z "$CONTAINER" ]; then
    BUILD_ARGS+="--build-arg DOCKER_CONTAINER=${CONTAINER}"
fi

DOCKER_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/; pwd)"
SOURCE_DIR=$(realpath "${DOCKER_DIR}/..")

echo
echo container $CONTAINER
echo tag $TAG
echo source $SOURCE_DIR
echo docker $DOCKER_DIR
echo

(
    cd ${SOURCE_DIR} || exit 1
    BUILDKIT_PROGRESS=plain docker build . -f "${DOCKER_DIR}/Dockerfile" ${BUILD_ARGS} --progress=plain -t $TAG
)
