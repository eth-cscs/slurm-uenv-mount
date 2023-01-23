#!/bin/bash

srun --uenv-file=/does/not/exist true |& grep 'Invalid squashfs image' || exit 1
