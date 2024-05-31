function setup() {
    bats_load_library bats-support
    bats_load_library bats-assert
    load ./common
    SQFSDIR=$(mktemp -d)
    export SQFSDIR
    dd=$(mktemp -d)
    (
        umask 022
        cd "$dd" || exit
        mkdir spack-install
        echo "This is file A" >> spack-install/fileA.txt
    )

    mksquashfs "$dd"  ${SQFSDIR}/binaries.sqfs  -quiet -noappend && rm -r "$dd"

    dd=$(mktemp -d)
    (
        umask 022
        cd "$dd" || exit
        mkdir profilers
        echo "profiler stack" >> profilers/fileB.txt
    )
    mksquashfs "$dd"  ${SQFSDIR}/profilers.sqfs -quiet  -noappend && rm -r "$dd"

    dd=$(mktemp -d)
    (
        umask 022
        cd "$dd" || exit
        mkdir tools
        echo "tools stack" >> tools/fileB.txt
    )
    mksquashfs "$dd"  ${SQFSDIR}/tools.sqfs -quiet -noappend && rm -r "$dd"
}

function teardown() {
    rm -r ${SQFSDIR}
}


@test "mount_stack_profiler_and_tools" {
    run_srun --uenv=${SQFSDIR}/binaries.sqfs,${SQFSDIR}/profilers.sqfs:/user-profilers,${SQFSDIR}/tools.sqfs:/user-tools true
}

@test "sbatch" {
    run_sbatch <<EOF
#!/bin/bash
#SBATCH --uenv=${SQFSDIR}/binaries.sqfs,${SQFSDIR}/profilers.sqfs:/user-profilers,${SQFSDIR}/tools.sqfs:/user-tools
srun findmnt /user-environment
srun findmnt /user-profilers
srun findmnt /user-tools
EOF
}

@test "sbatch_script_context" {
    run_sbatch <<EOF
#!/bin/bash
#SBATCH --uenv=${SQFSDIR}/binaries.sqfs,${SQFSDIR}/profilers.sqfs:/user-profilers,${SQFSDIR}/tools.sqfs:/user-tools
findmnt /user-environment
findmnt /user-profilers
findmnt /user-tools
EOF
}

@test "duplicate_flag" {
    # the second invocaton is ignored, resp. overwritten
    run_srun --uenv=${SQFSDIR}/binaries.sqfs --uenv=${SQFSDIR}/binaries.sqfs true
}

@test "plain" {
    # nothing is done if no --uenv is present and UENV_MOUNT_LIST is empty
    # TODO ideally one would check that the namespace before and in srun are the same
    unset UENV_MOUNT_LIST
    run_srun_unchecked  sh -c 'findmnt -r | grep /user-environment'
    refute_line --partial '/user-environment'
}

@test "uenv_mount_list_environment" {
    # check that if images have been mounted `uenv --uenv=...`, the slurm plugin recogines UENV_MOUNT_LIST and mounts the same images
    export UENV_MOUNT_LIST="${SQFSDIR}/binaries.sqfs,${SQFSDIR}/profilers.sqfs:/user-profilers,${SQFSDIR}/tools.sqfs:/user-tools"
    run_srun  sh -c 'findmnt /user-environment && findmnt /user-profilers && findmnt /user-tools'
}

@test "uenv_mount_list_environment_backward_compat" {
    # older versions of squashfs-mount used `file://` prefix for <abs-path> in UENV_MOUNT_LIST
    # check that if images have been mounted `uenv --uenv=...`, the slurm plugin recogines UENV_MOUNT_LIST and mounts the same images
    export UENV_MOUNT_LIST="file://${SQFSDIR}/binaries.sqfs,${SQFSDIR}/profilers.sqfs:/user-profilers,file://${SQFSDIR}/tools.sqfs:/user-tools"
    run_srun  sh -c 'findmnt /user-environment && findmnt /user-profilers && findmnt /user-tools'
}

@test "sbatch_override_in_srun" {
    # check that images mounted via sbatch --uenv are overriden when `--uenv` flag is given to srun
    run_sbatch <<EOF
#!/bin/bash
#SBATCH --uenv=${SQFSDIR}/binaries.sqfs

# override --uenv and mount under /user-tools instead
srun --uenv=${SQFSDIR}/binaries.sqfs:/user-tools findmnt /user-tools

# override, /user-environment must not be mounted
srun --uenv=${SQFSDIR}/binaries.sqfs:/user-tools bash -c '! findmnt /user-environment'
EOF
}

@test "plain_sbatch" {
    # check that images mounted via sbatch --uenv are overriden when `--uenv` flag is given to srun
    run_sbatch <<EOF
#!/bin/bash
srun true
EOF
}

@test "empty_uenv_mount_list" {
   UENV_MOUNT_LIST= srun true
}

@test "duplicate_mountpoints_fail" {
    run_srun_unchecked --uenv ${SQFSDIR}/binaries.sqfs,${SQFSDIR}/tools.sqfs true
    assert_output --partial 'Duplicate mountpoints found'
}

@test "duplicate_image_fails" {
    # duplicate images fail
    run_srun_unchecked --uenv ${SQFSDIR}/binaries.sqfs:/user-tools,${SQFSDIR}/binaries.sqfs:/user-profilers true
    assert_output --partial 'Duplicate images found'
}

@test "faulty_argument" {
    run_srun_unchecked --uenv=a:b:c:/user-tools true
    assert_output --partial 'Invalid syntax for --uenv'
}

# @test "empty_argument1" {
#     run_srun_unchecked --uenv='' true
#     assert_output --partial 'No mountpoints given.'
# }
