project('squashfs-mount', ['cpp'],
        default_options : ['cpp_std=c++17'],
        version: files('VERSION'), meson_version: '>=0.57')

version = meson.project_version()

dep_libmount = dependency('mount')

shared_module('slurm-uenv-mount',
              sources: ['src/plugin.cpp', 'src/mount.cpp'],
              dependencies: [dep_libmount],
              install: true)
