#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --output=/tmp/stdout.txt
#SBATCH --error=/tmp/stderr.txt

# if --uenv-mount-file is not specified, the plugin should not mount anything


# user-environment is mounted unexpectedly
set +e
(findmnt | grep /user-environment) && exit 1


# check the same in srun
srun sh <<\EOF
     set +e
     (findmnt | grep /user-environment) && exit 1
EOF


# this is expected to run silently
srun --uenv-mount-point=/foo sh <<\EOF
     set +e
     (findmnt | grep /user-environment) && exit 1
EOF
