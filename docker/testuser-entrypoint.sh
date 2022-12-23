#!/bin/bash

set -x

. /usr/lib64/mpi/gcc/mpich/bin/mpivars.sh

echo "To test plugin try:"
echo
echo "srun -t 10  --uenv-mount-file=fs.sqfs ls"
echo

if [ $# -gt 0 ];
then
   exec /entrypoint.sh su testuser -Ps /bin/bash -c "$*"
else
    exec /entrypoint.sh su testuser -s /bin/bash
fi
