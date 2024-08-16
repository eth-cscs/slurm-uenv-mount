#!/usr/bin/bash

CXX=g++-12 CC=gcc-12 meson setup builddir
meson install -C builddir
echo "required /usr/local/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf
# initialization
/entrypoint.sh
sinfo
echo "Run tests in bash-bats"
su testuser -c bash <<\EOF
bats ci/tests
EOF
