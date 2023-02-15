#!/bin/bash

set -e

mkdir -p /tmp/build-rpm
cd /tmp/build-rpm
make -f /slurm-uenv-mount/Makefile rpm
rpmbuild --rebuild "$(find rpm/SRPMS -name '*.rpm' -type f -print -quit)" --define "_topdir $(pwd)/rpm"
find rpm -name '*.rpm' -type f -exec cp {} . \;

echo
echo "RPM built. "
echo

echo "Copy from container:"
echo
printf "\tdocker compose cp slurm:$(realpath *x86_64.rpm) ."
echo
echo
