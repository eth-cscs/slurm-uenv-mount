
project('slurm-squashfs-mount', ['cpp'], default_options : ['cpp_std=c++17'], version: files('VERSION'), meson_version: '>=0.57')

dep_libmount = dependency('mount')

version = meson.project_version()

c_args = ['-DVERSION="@".format(version)]']

plugin = shared_module('plugin',
    sources: ['plugin.cpp'],
    dependencies: [dep_libmount],
    include_directories: ['slurm'],
    c_args: c_args,
    install: true)
