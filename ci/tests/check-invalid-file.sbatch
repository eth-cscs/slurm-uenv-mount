#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --output=/tmp/stdout.txt
#SBATCH --error=/tmp/stderr.txt

# user-environment is mounted unexpectedly
(findmnt | grep /user-environment) && exit 1

# check the same in srun
srun sh <<\EOF
     (findmnt | grep /user-environment) && exit 1
EOF
