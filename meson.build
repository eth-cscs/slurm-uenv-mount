project('slurm-uenv-mount', ['cpp'],
        default_options : ['cpp_std=c++17'],
        version: files('VERSION'), meson_version: '>=0.57')
version = meson.project_version()

default_mount_point = get_option('default_mount_point')
uenv_repo_path_varname = get_option('uenv_repo_path_varname')
conf_data = configuration_data()
conf_data.set('default_mount_point', default_mount_point)
conf_data.set('uenv_repo_path_varname', uenv_repo_path_varname)
configure_file(input : 'src/config.hpp.in',
               output : 'config.hpp',
               configuration : conf_data)

libmount_dep = dependency('mount')
sqlite3_dep = dependency('sqlite3')

shared_module('slurm-uenv-mount',
              sources: ['src/plugin.cpp',
                        'src/mount.cpp',
                        'src/parse_args.cpp',
                        'src/sqlite/sqlite.cpp',
                        'src/util/strings.cpp',
                        'src/util/filesystem.cpp'],
              dependencies: [libmount_dep, sqlite3_dep],
              cpp_args: ['-Wall', '-Wpedantic', '-Wextra'],
              install: true)
