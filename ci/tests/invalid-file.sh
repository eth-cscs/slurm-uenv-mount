#!/bin/bash

expect_error()
{
    (
        set +e
        if  ! (eval "$1") ; then
            return 0
        fi
        return 1
    )
}
export -f expect_error


expect_error 'srun -n 1 --uenv-file=/home/testuser/fs.sqfs --uenv-mount=/path/does/not/exist true'
