# slurm-uenv-mount

## Usage

```
srun --uenv-mount-file=/path/to/store.sqfs  [--uenv-mount-point=/path/to/mountpoint/] [--uenv-skip-prologue] CMD
```

IF the --unev-mount-file flag is missing, but environment variable `SLURM_UENV_MOUNT_FILE` is set, will mount the specified file.
