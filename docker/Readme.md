# Docker container to run the plugin for development purposes

Build a container named `slurm-uenv-mount`:
```bash
./build.sh
```

Run the container as unprivilged user (`testuser`):
```bash
docker run --name slurm-uenv-mount --rm --privileged -it slurm-uenv-mount bash
```

 `run.sh` can be used as a shortcut:
```bash
./run.sh bash
```

`./rebuild.sh` rebuilds the plugin in the container from the local source tree (this assumes the container was stareted via `./run.sh`).
