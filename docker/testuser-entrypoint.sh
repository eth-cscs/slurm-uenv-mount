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

echo "To test plugin try:"
echo
echo "srun -t 10  --uenv-mount-file=fs.sqfs ls"
echo

if [ $# -gt 0 ];
then
   exec /entrypoint.sh su testuser -s /bin/bash -c "$@"
else
    exec /entrypoint.sh su testuser -s /bin/bash
fi
