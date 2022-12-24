#!/bin/bash

echo "--------------------------------"
echo "Test --uenv-mount-file in sbatch"
echo "--------------------------------"
sbatch --wait /slurm-uenv-mount/ci/tests/test.sbatch

echo "-----------------------------------------------------------------------"
echo "test inheritance of UENV_MOUNT_FILE/POINT in dummy squashfs-run session"
echo "-----------------------------------------------------------------------"
/slurm-uenv-mount/ci/tests/squashfs-run.sh
