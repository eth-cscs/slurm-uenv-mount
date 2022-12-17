#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --uenv-mount-file=/fs.sqfs
#SBATCH --output=/tmp/stdout.txt
#SBATCH --error=/tmp/stderr.txt

echo "from inside sbatch.sh: $(stat /user-environment/test)"

srun -n 2 --uenv-mount-file=/fs.sqfs  sh <<\EOF
        echo "from inside srun node ${SLURM_NODEID}, task ${SLURM_TASK_PID}: $(stat /user-environment/test)"
EOF
