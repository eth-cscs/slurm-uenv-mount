#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
. ${SCRIPT_DIR}/util

expect_error 'srun -n 1 --uenv-file=/home/testuser/fs.sqfs --uenv-mount=/path/does/not/exist true'
