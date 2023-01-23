#!/bin/bash

# non-existent mount point
(srun -n 1 --uenv-mount=/user-environment true) |& grep 'uenv-mount is only allowed to be used together with --uenv-file' || exit 1
