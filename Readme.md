# WIP rootless plugin for slurm-uenv-mount

## Usage

```
srun --uenv-mount-file=/path/to/store.sqfs  [--uenv-mount-point=/path/to/mountpoint/] [--uenv-skip-prologue] CMD
```

If an active squashfs-mount session is detected, meaning both
`SLURM_UENV_MOUNT_FILE` and `SLURM_UENV_MOUNT_POINT` environment variables are
set, the plugin will consider those variables unless explicitly overriden by the
`--uenv-mount-file`, `--uenv-mount-point` flag. The flags given to the plugin,
for example in #SBATCH are inherited by subsequent srun commands (unless
overridden). They behave as standard slurm flags `-N` etc.

*NOTE* Combining the the slurm-uenv-mount spank plugin with the old workflow, e.g. `srun --uenv-mount-file=store.sqfs squashfs-run CMD` will fail.
