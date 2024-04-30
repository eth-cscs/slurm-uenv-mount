function setup() {
    bats_load_library bats-support
    bats_load_library bats-assert
    load ./common
    SQFSDIR=$(mktemp -d)
    export SQFSDIR
    rm -f index.db
    sqlite3 ${SQFSDIR}/index.db < index.db.txt

    img_hashes=(1736b4bb5ad9b3c5cae8878c71782a8bf2f2f739dbce8e039b629de418cb4dab 3313739553fe6553f789a35325eb6954a37a7b85cdeab943d0878a05edaac998 3e8f96370a4685a7413d344d98f69889c0ba6bb1d6c2d3d19ce01b6079c58c68)

    for hash in ${img_hashes[@]}
    do
        mkdir -p ${SQFSDIR}/images/$hash
    done

    dd=$(mktemp -d)
    (
        umask 022
        cd "$dd" || exit
        mkdir spack-install
        echo "This is file A" >> spack-install/fileA.txt
    )
    mksquashfs "$dd"  ${SQFSDIR}/store.squashfs  -quiet -noappend && rm -r "$dd"

    for hash in ${img_hashes[@]}
    do
        cp ${SQFSDIR}/store.squashfs  ${SQFSDIR}/images/${hash}/
    done
}

function teardown() {
    rm -r ${SQFSDIR}
}


@test "mount prgenv-gnu" {
    export UENV_REPO_PATH=${SQFSDIR}
    run_srun --uenv=prgenv-gnu/24.2 true
    run_srun --uenv=prgenv-gnu:latest true
    run_srun --uenv=prgenv-gnu/24.2:latest true
}

@test "mount jfrog image by id" {
    export UENV_REPO_PATH=${SQFSDIR}
    run_srun --uenv=1736b4bb5ad9b3c5 true
}

@test "mount jfrog image by sha256" {
    export UENV_REPO_PATH=${SQFSDIR}
    run_srun --uenv= 1736b4bb5ad9b3c5cae8878c71782a8bf2f2f739dbce8e039b629de418cb4dab true
}

@test "attempt to mount ambiguous prgenv-gnu" {
    export UENV_REPO_PATH=${SQFSDIR}
    run_srun_unchecked --uenv=prgenv-gnu true
    assert_output --partial 'More than one uenv matches.'
}


# @test "plain" {
#     # nothing is done if no --uenv is present and UENV_MOUNT_LIST is empty
#     # TODO ideally one would check that the namespace before and in srun are the same
#     unset UENV_MOUNT_LIST
#     run_srun_unchecked  sh -c 'findmnt -r | grep /user-environment'
#     refute_line --partial '/user-environment'
# }

# @test "uenv_mount_list_environment" {
#     # check that if images have been mounted `uenv --uenv=...`, the slurm plugin recogines UENV_MOUNT_LIST and mounts the same images
#     export UENV_MOUNT_LIST="${SQFSDIR}/binaries.sqfs,${SQFSDIR}/profilers.sqfs:/user-profilers,${SQFSDIR}/tools.sqfs:/user-tools"
#     run_srun  sh -c 'findmnt /user-environment && findmnt /user-profilers && findmnt /user-tools'
# }

# @test "sbatch_override_in_srun" {
#     # check that images mounted via sbatch --uenv are overriden when `--uenv` flag is given to srun
#     run_sbatch <<EOF
# #!/bin/bash
# #SBATCH --uenv=${SQFSDIR}/binaries.sqfs

# # override --uenv and mount under /user-tools instead
# srun --uenv=${SQFSDIR}/binaries.sqfs:/user-tools findmnt /user-tools

# # override, /user-environment must not be mounted
# srun --uenv=${SQFSDIR}/binaries.sqfs:/user-tools bash -c '! findmnt /user-environment'
# EOF
# }

# @test "duplicate_mountpoints_fail" {
#     run_srun_unchecked --uenv ${SQFSDIR}/binaries.sqfs,${SQFSDIR}/tools.sqfs true
#     assert_output --partial 'Duplicate mountpoints found'

# }

# @test "duplicate_image_fails" {
#     # duplicate images fail
#     run_srun_unchecked --uenv ${SQFSDIR}/binaries.sqfs:/user-tools,${SQFSDIR}/binaries.sqfs:/user-profilers true
#     assert_output --partial 'Duplicate images found'
# }

# @test "faulty_argument" {
#     run_srun_unchecked --uenv=a:b:c:/user-tools true
#     assert_output --partial 'Invalid syntax for --uenv'
# }

# @test "empty_argument1" {
#     run_srun_unchecked --uenv='' true
#     assert_output --partial 'No mountpoints given.'
# }

# @test "empty_argument2" {
#     run_srun_unchecked --uenv=,,, true
#     assert_output --partial 'No mountpoints given.'
# }
