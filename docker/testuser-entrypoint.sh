#!/bin/bash

. /usr/lib64/mpi/gcc/mpich/bin/mpivars.sh

exec /entrypoint.sh su testuser -s /bin/bash -l
