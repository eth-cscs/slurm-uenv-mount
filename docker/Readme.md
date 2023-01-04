# Docker container to run the plugin for development purposes

Build the container:
```bash
docker compose build
```

Start the container (named `slurm-uenv-mount`):
```bash
docker compose up -d
```
- Runs container in the background.
- The source tree is mounted read-only under `/slurm-uenv-mount`.
- Interact with the container using `docker exec -it slurm-uenv-mount ...`.

Launch a shell in the container as unprivileged user:
```bash
docker exec -u testuser -w /home/testuser -it slurm-uenv-mount bash
```

Run tests:
```bash
docker exec -u testuser -w /home/testuser -i slurm-uenv-mount bash < run-tests.sh
```

When making changes in the local source tree, the plugin can be rebuilt/resintalled in the running container (`docker compose up`) via:
```bash
./rebuild.sh
```
