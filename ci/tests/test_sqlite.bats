function setup() {
    bats_load_library bats-support
    bats_load_library bats-assert
    load ./common
    SQFSDIR=$(mktemp -d)
    export SQFSDIR
    rm -f index.db
    sqlite3 ${SQFSDIR}/index.db < ci/tests/index.db.txt

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
    run_srun --uenv=1736b4bb5ad9b3c5cae8878c71782a8bf2f2f739dbce8e039b629de418cb4dab true
}

@test "attempt to mount ambiguous prgenv-gnu" {
    export UENV_REPO_PATH=${SQFSDIR}
    run_srun_unchecked --uenv=prgenv-gnu true
    assert_output --partial 'More than one uenv matches.'
}


