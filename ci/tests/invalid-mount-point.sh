#!/bin/bash

# non-existent mount point
(srun -n 2 --uenv-mount-file=/home/testuser/fs.sqfs --uenv-mount-point=/path/does/not/exist true) |& grep "Invalid mount point" && exit 1

# non-existent file
(srun -n 2 --uenv-mount-file=/home/testuser ) |& grep "Invalid squashfs image" && exit 1
