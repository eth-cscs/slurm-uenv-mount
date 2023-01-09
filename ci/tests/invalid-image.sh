#!/bin/bash

srun --uenv-mount-file=/does/not/exist true |& grep 'Invalid squashfs image' || exit 1
