SLURM_VERSION="20.11.9"
slurm_tar_file=slurm-${SLURM_VERSION}.tar.bz2
slurm_url=https://download.schedmd.com/slurm/${slurm_tar_file}

echo "=== downloading slurm ${SLURM_VERSION} from ${slurm_url}"

if [ -d slurm ]
then
    rm -rf slurm
fi

curl --output ${slurm_tar_file} --silent ${slurm_url}

echo "=== unpacking $slurm_tar_file"
tar -xjf ${slurm_tar_file}

echo "=== moving to slurm path"
mv slurm-${SLURM_VERSION} slurm

echo "=== updating slurm headers"
for s in slurm/slurm/*.in
do
    mv $s ${s::-3}
done
