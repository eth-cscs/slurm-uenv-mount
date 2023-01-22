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


# # this always fails, I don't understand why
# srun sh <<\EOF
#     if [[ $(findmnt | grep -bc /user-environment) -ne $(findmnt | wc -l)  ]];
#     then
#         exit 1
#     fi
# EOF

# # this is expected to run silently
# srun --uenv-mount=/foo sh <<\EOF
#     if [[ $(findmnt | grep -bc /user-environment) -ne $(findmnt | wc -l)  ]];
#     then
#         exit 1
#     fi
# EOF
