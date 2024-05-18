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

lib_src = ['src/lib/parse_args.cpp',
           'src/lib/database.cpp',
           'src/lib/filesystem.cpp',
           'src/lib/mount.cpp',
           'src/lib/sqlite.cpp',
           'src/lib/strings.cpp']

module_src = ['src/plugin.cpp'] + lib_src

module_inc = include_directories('src')

module_dep = [libmount_dep, sqlite3_dep]

shared_module('slurm-uenv-mount',
              sources: module_src,
              dependencies: module_dep,
              include_directories: module_inc,
              cpp_args: ['-Wall', '-Wpedantic', '-Wextra'],
              install: true)

unit_inc = include_directories('external/catch2')

unit_src = ['external/catch2/catch_amalgamated.cpp',
            'tests/unit/parse_args.cpp',
            'tests/unit/strings.cpp',]
unit_src += lib_src

unit = executable('unit',
                  sources: unit_src,
                  include_directories: [unit_inc, module_inc],
                  dependencies: module_dep,
                  build_by_default: true,
                  install: false)
