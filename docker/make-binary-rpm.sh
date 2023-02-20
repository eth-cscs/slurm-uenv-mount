#!/bin/bash

set -e

mkdir -p /tmp/build-rpm
cd /tmp/build-rpm

_SLURM_VER=$(srun --version | sed 's/slurm //')
make -f /slurm-uenv-mount/Makefile RPM_SLURM_VERSION="${_SLURM_VER}" rpm
rpmbuild --rebuild "$(find rpm/SRPMS -name '*.rpm' -type f -print -quit)" --define "_topdir $(pwd)/rpm"

rm ./*.rpm
find rpm -name '*.rpm' -type f -exec cp {} . \;

fname=$(ls *x86_64.rpm)
dstname=$(echo $fname | sed "s/x86_64/x86_64-slurm-${_SLURM_VER}/")
mv $fname $dstname


echo
echo "RPM built. "
echo

echo "Copy from container:"
echo
printf "\tdocker compose cp slurm:$(realpath $dstname) ."
echo
echo
