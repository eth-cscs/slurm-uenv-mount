#!/bin/bash

# pretend to run inside squasfsh-run ... bash
export UENV_MOUNT_FILE=/home/testuser/fs.sqfs
export UENV_MOUNT_POINT=/tmp

srun stat /tmp/test/fileA.txt

# use --uenv-mount-file to override UENV_MOUNT_FILE
# expectation: squahfs is mounted in `/user-environment/` (default location)
srun --uenv-mount-file=/home/testuser/fs2.sqfs stat /user-environment/test/fileB.txt
