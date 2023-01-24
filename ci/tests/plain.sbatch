#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --output=/tmp/stdout.txt
#SBATCH --error=/tmp/stderr.txt

# if --uenv-file is not specified, the plugin should not mount anything

# user-environment is mounted unexpectedly
if [[ $(findmnt | grep -vc /user-environment) -ne $(findmnt | wc -l)  ]];
then
    exit 1
fi
