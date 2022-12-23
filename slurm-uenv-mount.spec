Name:           slurm-uenv-mount
Version:        UENVMNT_VERSION
Release:        1%{?dist}
Summary:        SLURM spank plugin to mount squashfs images.
Prefix:         /usr

License:        BSD3
URL:            https://github.com/eth-cscs/slurm-uenv-mount
Source0:        %{name}-%{version}.tar.gz

%define _build_id_links none

%description
A SLURM spank plugin to mount squashfs images.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
make install prefix=%{_prefix} DESTDIR=%{buildroot}

%post
REQ="required /usr/lib64/libslurm-uenv-mount.so"
CNF=/etc/plugstack.conf.d/99-slurm-uenv-mount.conf
echo "$REQ" >> "$CNF"

%files
%license LICENSE
%{_libdir}/lib%{name}.so
