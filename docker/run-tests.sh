#!/bin/bash

set -e

run_test_script()
{
    $1 < /dev/null || (echo "$1 failed" && exit 1)
    echo "-> $1 passed"
}

run_sbatch()
{
    printf "\tsbatch %s" "$1"
    echo
    sbatch --wait "$1" || exit 1
    echo "-> $1 passed"
}

(
    # while IFS= read -r -d '' file
    # do
    #     run_sbatch "${file}"
    # done <   <(find /slurm-uenv-mount/ci/tests -iname '*.sbatch' -print0)

    # echo "-----------------------------------------------------------------------"
    # echo "test inheritance of UENV_MOUNT_FILE/POINT in dummy squashfs-run session"
    # echo "-----------------------------------------------------------------------"
    # run_test_script /slurm-uenv-mount/ci/tests/squashfs-run.sh

    # echo "-----------------------------------------------------------------------"
    # echo "invalid.image.sh (expectation: slurm error)                            "
    # echo "-----------------------------------------------------------------------"
    # run_test_script /slurm-uenv-mount/ci/tests/invalid-image.sh

    # echo "-----------------------------------------------------------------------"
    # echo "invalid-file.sh (expectation: slurm error)                             "
    # echo "-----------------------------------------------------------------------"
    # run_test_script /slurm-uenv-mount/ci/tests/invalid-file.sh

    # echo "-----------------------------------------------------------------------"
    # echo "invalid-flags.sh (expectation: slurm error)                             "
    # echo "-----------------------------------------------------------------------"
    # run_test_script /slurm-uenv-mount/ci/tests/invalid-flags.sh

    # echo "-----------------------------------------------------------------------"
    # echo "relative paths (check)                                                 "
    # echo "-----------------------------------------------------------------------"
    # (
    #     set -x
    #     sbatch /slurm-uenv-mount/ci/tests/relative-path.sbatch
    # )

    echo "-----------------------------------------------------------------------"
    echo "relative paths                                                         "
    echo "-----------------------------------------------------------------------"
    (
        set -x
        script=$(realpath /slurm-uenv-mount/ci/tests/relative-path.sbatch)
        echo "script: ${script}"
        cd ~
        sbatch --wait $script
    )

) || (printf 'TESTS FAILED' && exit 1)

echo
echo
printf " \xE2\x9C\x94 ALL TESTS PASSED"
echo
echo
