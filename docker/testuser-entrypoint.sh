#!/bin/bash

# Switch to the user "testuser" to run the rest of the script
if [ "$UID" = "0" ]; then
    cat /var/log/slurm/slurmctld.log
    cat /var/log/slurm/slurmd.nd00001.log
    cat /var/log/slurm/slurmd.nd00002.log
    cat /var/log/slurm/slurmd.nd00003.log
    su -l --whitelist-environment=SLURM_ROOT -s /bin/bash testuser $0 $*
    exit
fi


. /usr/lib64/mpi/gcc/mpich/bin/mpivars.sh

export PATH=$SLURM_ROOT/bin:$PATH
export LD_LIBRARY_PATH=$SLURM_ROOT/lib:$SLURM_ROOT/lib64:$PATH
export MANPATH=$SLURM_ROOT/share/man:$PATH


dd=$(mktemp -d)

(
    umask 022
    cd "$dd" || exit
    mkdir test
    echo "This is file A" >> test/fileA.txt
    mkdir -p test/testdir
)

if command -v tree
then
    tree "$dd"
fi

mksquashfs  "$dd"  $HOME/fs.sqfs -noappend && rm -r "$dd"


echo "To test plugin try:"
echo
echo "srun -t 10  --uenv-mount-file=0.1.105.tar.gz ls"
echo

exec $SHELL
