Name:           slurm-uenv-mount
Version:        UENVMNT_VERSION
Release:        SLURM_VERSION
Summary:        SLURM spank plugin to mount squashfs images.
Prefix:         /usr
Requires:       slurm = RPM_SLURM_VERSION

License:        BSD3
URL:            https://github.com/eth-cscs/slurm-uenv-mount
Source0:        %{name}-%{version}.tar.gz

BuildRequires: meson gcc slurm-devel

%define _build_id_links none

%description
A SLURM spank plugin to mount squashfs images.

%prep
%autosetup -c

%build
%meson
%meson_build

%install
%meson_install

%post
REQ="required /usr/lib64/libslurm-uenv-mount.so"
CNF=/etc/plugstack.conf.d/99-slurm-uenv-mount.conf
mkdir -p /etc/plugstack.conf.d
echo "$REQ" > "$CNF"

%files
%license %{_vpath_srcdir}/LICENSE
%{_libdir}/lib%{name}.so
