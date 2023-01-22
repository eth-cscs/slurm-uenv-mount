#!/bin/bash
#SBATCH --ntasks=2
#SBATCH --uenv-file=/home/testuser/fs.sqfs
#SBATCH --uenv-mount=/user-environment

# check that --uenv-file is inherited from the #SBATCH argument
srun -n 2 sh <<\EOF
        echo "from inside srun node ${SLURM_NODEID}, task ${SLURM_TASK_PID}: $(stat /user-environment/test)"
EOF

# override image
srun -n 2 --uenv-file=/home/testuser/fs2.sqfs sh <<\EOF
        echo "from inside srun node ${SLURM_NODEID}, task ${SLURM_TASK_PID}: $(stat /user-environment/test/fileB.txt)"
EOF

# override mountpoint
mkdir -p /tmp/uenv
srun -n 2 --uenv-mount=/tmp/uenv sh <<\EOF
        echo "from inside srun node ${SLURM_NODEID}, task ${SLURM_TASK_PID}: $(stat /tmp/uenv/test)"
EOF
