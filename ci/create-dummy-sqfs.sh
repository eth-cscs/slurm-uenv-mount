#!/bin/bash

(
    set -e
    cd /home/testuser || exit
    su testuser -c sh <<\EOF
dd=$(mktemp -d)
(
    umask 022
    cd "$dd" || exit
    mkdir test
    echo "This is file A" >> test/fileA.txt
    mkdir -p test/testdir
)

mksquashfs  "$dd"  /home/testuser/fs.sqfs -noappend && rm -r "$dd"

dd=$(mktemp -d)
(
    umask 022
    cd "$dd" || exit
    mkdir test
    echo "This is file A" >> test/fileB.txt
    mkdir -p test/testdir
)
mksquashfs  "$dd"  /home/testuser/fs2.sqfs -noappend && rm -r "$dd"
EOF
)
