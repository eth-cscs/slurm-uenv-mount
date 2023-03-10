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

Build the container (container name `slurm-uenv-mount`, service name `slurm`):
```bash
docker compose build
```

By default the plugin is built in a container with slurm `20.11.9`. The version
can be changed in `docker-compose.yaml` (check
github.com/eth-cscs/slurm-container for a list of available versions).

Start the container in the background:
```bash
docker compose up -d
```
- The source tree is mounted read-only under `/slurm-uenv-mount`.

Launch a shell in the container as unprivileged user:
```bash
docker compose exec --privileged -u testuser -w /home/testuser slurm bash
```

Run tests:
```bash
docker compose exec --privileged -u testuser -w /home/testuser -T slurm bash < run-tests.sh
```

The source code is mounted read-only inside the container under `/slurm-uenv-mount`.
When making changes in the local source tree, the plugin can be rebuilt/resintalled in the running container with the following script:
```bash
./rebuild.sh
```

Build the binary rpm, e.g. for installation on production system:
```bash
docker compose exec -T slurm bash < make-binary-rpm.sh
```
