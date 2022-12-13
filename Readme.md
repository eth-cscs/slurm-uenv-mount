# slurm-uenv-mount

## Usage

```
srun --uenv-mount-file=/path/to/store.sqfs  [--uenv-mount-point=/user-environment] CMD
```

If an active squashfs-mount session is detected, the plugin will mount the same
squashfs image (spack stack) unless overridden by the `--uenv-mount-file` flag. In the
latter case the squashfs-image will be mounted under `/user-environment` or the
mount point specified via `--uenv-mount-point`.

The command line flags given to the plugin, for example in #SBATCH, are inherited by subsequent srun
commands (unless overridden). They behave as standard slurm flags, for example `-N,--nodes` etc.

**NOTE** Combining the the slurm-uenv-mount spank plugin with the old workflow, e.g. `srun --uenv-mount-file=store.sqfs squashfs-run CMD` doesn't work.
