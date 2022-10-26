project('squashfs-mount', ['c', 'cpp'], default_options : ['c_std=c99', 'cpp_std=c++17'], version:
files('VERSION'), meson_version: '>=0.57')
version = meson.project_version()

# Look for libmount, squashfuse_ll
dep_libmount = dependency('mount')


shared_module('sqfs-spank-plugin',
              sources: ['plugin.cpp', 'mount.cpp'],
              dependencies: [dep_libmount],
              install: true)
