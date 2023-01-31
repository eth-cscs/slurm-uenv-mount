#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
. ${SCRIPT_DIR}/util

expect_error 'srun --uenv-mount=/user-environment -v true'
expect_error 'srun --uenv-file=/home/testuser/fs.sqfs --uenv-mount=/user-environment -v true'
