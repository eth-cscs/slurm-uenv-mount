# Docker container to run the plugin for development purposes

To test a slurm plugin a running slurm server and clients are needed. Rather
than require such a setup on the development platform itself, the CSCS Slurm
Docker container can be used. This provides a single server instance and
multiple client daemons. 

Pull the container that runs a slurm server and clients. The container is
hosted on GitHub Packages. To access this server you will need a GitHub Access
Token (https://github.com/settings/tokens) with at least the read:packages scope.
```bash
docker login --username=<USERNAME> ghcr.io 
```
When using GitHub Packages, `USERNAME` can be any text. The password is the GitHub Token.

Next pull the container from GitHub Packages.
```bash
docker pull <DOCKER_CONTAINER>
```
`DOCKER_CONTAINER` is the full name of the slurm container. This will typically
be something like `ghcr.io/eth-cscs/slurm-container-SLURM_VERSION:latest`.
For instance,
```bash
docker pull ghcr.io/eth-cscs/slurm-container-20.11.9:latest
```

Now build a container named `slurm-uenv-mount` that contains the Slurm server, clients, and the plugin.
The `Dockerfile` in this directory needs to use the same `DOCKER_CONTAINER` pulled earlier.
Either edit `Dockerfile` and replace the name of the default root container or supply the name on the commandline.
```bash
./build.sh [DOCKER_CONTAINER]
```

The Dockerfile creates user `testuser` for running slurm commands as a normal user. The container itself should run
as privileged to allow the slurm server to start.

Run the container and start an interactive session as the unprivileged user `testuser`:
```bash
docker run --name slurm-uenv-mount --rm --privileged -it slurm-uenv-mount bash
```

Alternatively, `run.sh` can be used as a shortcut:
```bash
./run.sh bash
```

`./rebuild.sh` rebuilds the plugin in the container from the local source tree (this assumes the container was started via `./run.sh`).

# Rebuilding the plugin

The `run.sh` script mounts the source code directory as a read-only volume inside the container under `/slurm-uenv-mount`.
This allows the source code to be modified outside of the container, but rebuilt and tested in a slurm environment. 
To rebuild the plugin, first create a build directory in the `testuser` home inside a running container.
Then run `make` from the build directory, specifying the Makefile from the source directory.
```bash
mkdir BUILD
cd BUILD
make -f /slurm-uenv-mount/Makefile
```

Similarly, the source RPM can be build by specifying the `rpm` target.
```bash
make -f /slurm-uenv-mount/Makefile rpm
```

