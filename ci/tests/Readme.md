# Tests

| Filename                     | Expectation                                                                                                                   |
|:-----------------------------|-------------------------------------------------------------------------------------------------------------------------------|
| `plain.sbatch`               | nothing is mounted when running sbatch (no uenv flags)                                                                        |
| `squashfs-run.sh`            | - env variables from squashfs-run are taken into account <br> - env variables have no effect when --uenv-mount-file overrides |
| `test-override-flags.sbatch` | override flags take precedence                                                                                                |
| `test.sbatch`                | image is mounted under correct mount point / content is accessible                                                            |
| `uenv-mount-point-alone.sh`  | plugin fails when  --uenv-mount-point is specified alone                                                                      |
