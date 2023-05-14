#!/bin/bash

set -e

mkdir -p /tmp/build-rpm
cd /tmp/build-rpm

_SLURM_VER=$(srun --version | sed 's/slurm //')

/slurm-uenv-mount/rpm/prepare-rpm.sh --slurm-version "${_SLURM_VER}" /tmp/src-rpm

find /tmp/src-rpm -name '*.rpm' -type f -exec cp {} . \;

fname=$(ls *x86_64.rpm)


echo
echo "RPM built. "
echo

echo "Copy from container:"
echo
printf "\tdocker compose cp slurm:$(realpath $fname) ."
echo
echo
