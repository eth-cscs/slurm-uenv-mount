#!/bin/bash

(
    set -e
    cd /home/testuser || exit
    su testuser -c sh <<\EOF
dd=$(mktemp -d)
(
    umask 022
    cd "$dd" || exit
    mkdir spack-install
    echo "This is file A" >> test/fileA.txt
    mkdir -p test/testdir
)

mksquashfs  "$dd"  /home/testuser/binaries.sqfs -noappend && rm -r "$dd"

dd=$(mktemp -d)
(
    umask 022
    cd "$dd" || exit
    mkdir profilers
    echo "profiler stack" >> profilers/fileB.txt
)
mksquashfs  "$dd"  /home/testuser/profilers.sqfs -noappend && rm -r "$dd"

dd=$(mktemp -d)
(
    umask 022
    cd "$dd" || exit
    mkdir tools
    echo "tools stack" >> tools/fileB.txt
)
mksquashfs  "$dd"  /home/testuser/tools.sqfs -noappend && rm -r "$dd"
EOF

)
