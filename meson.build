project('squashfs-mount', ['cpp'], default_options : ['cpp_std=c++17'], version:
files('VERSION'), meson_version: '>=0.57')
version = meson.project_version()

shared_module('slurm-uenv-mount',
              sources: ['plugin.cpp'],
              install: true)
