#!/bin/bash

# non-existent mount point
(srun -n 1 --uenv-mount-point=/user-environment true) |& grep 'uenv-mount-point is only allowed to be used together with --uenv-mount-file' || exit 1
