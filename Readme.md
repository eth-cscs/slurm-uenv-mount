# slurm-uenv-mount

The uenv-mount SLURM plugin mounts a file-system image on each compute node
(under `/user-environment` by default) when a SLURM job is launched. The image
is typically used to provide user environment (uenv) software, such as common
tools, libraries, and compilers.  Currently, only SquashFS images are
supported.

## Dependencies

A [slurm](https://slurm.schedmd.com/) installation.

## Usage

The plugin is activated by passing the option `--uenv` to any of the standard SLURM job submission
commands (e.g., `sbatch`, `srun`, `salloc`). This option accepts a comma-separated list of
squashfs image and mountpoint pairs:

```bash
srun --uenv=<file>:[<mountpoint>] CMD
srun --uenv=<file>:[<mountpoint>],<file>:[<mountpoint>]... CMD
```

`<file>` must be a SquashFS image. `<file>` and `<mountpoint>` must be
absolute paths. `<mountpoint>` defaults to `/user-environment` if omitted.

Multiple images and mount points can be passed as a comma separated list:
```bash
srun --uenv=/scratch/binaries.sqfs,/scratch/profilers.sqfs:/user-profilers CMD
```

Command line flags given to the plugin, for example via `#SBATCH`, are inherited
by subsequent `srun` commands (unless overridden).

If a user environment is already active, the file and mount point will be
inherited by subsequent SLURM command unless explicitly overridden. If a
different mount point is desired, the file must also be given as an argument. A
user environment is active if loaded with the `squashfs-mount` command or this
plugin.

**NOTE** Combining the the slurm-uenv-mount spank plugin with the old workflow, e.g. `srun --uenv=store.sqfs squashfs-run CMD` does not work.


### Examples

```bash
srun --uenv=/path/to/store.sqfs:/user-environment CMD
```

```bash
#SBATCH --uenv=/path/to/store.sqfs

# store.sqfs mounted on /user-environment
srun CMD

# another-store.sqfs mounted on /different-user-environment. Nothing mounted on /user-environment(!)
srun --uenv=/path/to/another-store.sqfs:/different-user-environment CMD
```

## Building the plugin

Meson is used as build system. Run `meson setup builddir`, `meson compile -C
builddir`, or `meson install -C builddir` to install `libslurm-uenv-mount.so`.

## Manual Installation

To enable the plugin, SLURM needs to know where the library is located. This is
provided via the file named by the `PlugStackConfig` option in `slurm.conf`. Typically,
this file is called `plugstack.conf` and is located in the same directory as `slurm.conf`.

For example, one can append to the file `/etc/slurm/plugstack.conf` the line
```
required /usr/lib64/libslurm-uenv-mount.so
```

See [slurm.conf](https://slurm.schedmd.com/slurm.conf.html#OPT_PlugStackConfig) or
[spank](https://slurm.schedmd.com/spank.html#SECTION_CONFIGURATION) documentation for details.


## Redistribution

A script to create an rpm package is provided in `rpm/make-rpm.sh`:

```bash
    tmpdir=$(mktemp -d)
    # create rpm directory structure / srpm in $tmpdir and compile the binary rpm
    rpm/make-rpm.sh ${tmpdir}
```

See `make-rpm.sh -h` for more options.
