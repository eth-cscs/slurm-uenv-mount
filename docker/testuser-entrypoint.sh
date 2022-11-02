#!/bin/bash

set -x

. /usr/lib64/mpi/gcc/mpich/bin/mpivars.sh

2>&1 > /dev/null su testuser -c sh <<\EOF
dd=$(mktemp -d)

(
    umask 022
    cd "$dd" || exit
    mkdir test
    echo "This is file A" >> test/fileA.txt
    mkdir -p test/testdir
)

if command -v tree
then
    tree "$dd"
fi

mksquashfs  "$dd"  /home/testuser/fs.sqfs -noappend && rm -r "$dd"
EOF

exec /entrypoint.sh su testuser -s /bin/bash -l
