#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --uenv-file=./fs.sqfs
#SBATCH --output=/tmp/stdout.txt
#SBATCH --error=/tmp/stderr.txt

cd /var/log || exit

echo "in local context: $(stat /user-environment/test)"
srun -n 1 sh <<\EOF
   echo "from inside srun node ${SLURM_NODEID}, task ${SLURM_TASK_PID}: $(stat /user-environment/test)"
EOF
