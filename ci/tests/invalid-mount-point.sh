#!/bin/bash

# non-existent mount point
(srun -n 1 --uenv-mount-file=/home/testuser/fs.sqfs --uenv-mount-point=/path/does/not/exist true) |& grep "Invalid mount point" || exit 1
