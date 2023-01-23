#!/bin/bash

# non-existent mount point
(srun -n 1 --uenv-file=/home/testuser/fs.sqfs --uenv-mount=/path/does/not/exist true) |& grep "Invalid mount point" || exit 1
