#!/bin/bash

set -xe

echo "for loop"
echo "$(find /slurm-uenv-mount/ci/tests -iname '*.sbatch')"
for sbatch_input in $(find /slurm-uenv-mount/ci/tests -iname '*.sbatch');
do
    echo "sbatch --wait ${sbatch_input}"
    sbatch --wait "${sbatch_input}"
done

while IFS= read -r -d '' file
do
    sbatch --wait "${file}"

done <   <(find /slurm-uenv-mount/ci/tests -iname '*.sbatch' -print0)


echo "-----------------------------------------------------------------------"
echo "test inheritance of UENV_MOUNT_FILE/POINT in dummy squashfs-run session"
echo "-----------------------------------------------------------------------"
/slurm-uenv-mount/ci/tests/squashfs-run.sh
