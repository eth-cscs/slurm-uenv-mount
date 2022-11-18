#!/bin/bash

set -e
#set -x

# By default use the current path as the root for source and build.
# This can be configured using command line args for out of tree builds.
source_path="$PWD"
build_path="$PWD/rpmbuild"
#while getopts s:b: flag
options=$(getopt -l "src:,build:,pkgname:,spec-in:,files:" -o "" -- "$@")

eval set -- "$options"
while true
do
    flag=$1 ; arg=$2
    #echo ${flag}
    case "${flag}" in
        --src)     shift; source_path=${arg};;
        --build)   shift; build_path=${arg};;
        --pkgname) shift; pkg_name=${arg};;
        --spec-in) shift; spec_in_file=${arg};;
	--files)   shift; files=(${arg});;
	--)        shift; break;;
        *) echo "unknown flag ${flag}"; exit 1 ;;
    esac
    shift
done

mkdir -p "${build_path}"
(cd "${build_path}" && mkdir -p SOURCES BUILD RPMS SRPMS SPECS)

tar_path="${build_path}/${pkg_name}"
mkdir -p "${tar_path}"

#files=(plugin.cpp mount.hpp mount.cpp Makefile LICENSE VERSION)

for f in "${files[@]}"
do
    cp "${source_path}/${f}" "${tar_path}"
done


tar_file="${build_path}/SOURCES/${pkg_name}.tar.gz"
tar -czf "${tar_file}" --directory "${build_path}" "${pkg_name}"

spec_file="${build_path}/SPECS/$(basename ${spec_in_file%.in})"
cp "${spec_in_file}" "${spec_file}"
