ARG DOCKER_CONTAINER 
FROM ${DOCKER_CONTAINER}
#FROM ghcr.io/eth-cscs/slurm-container-20.11.9:latest

RUN zypper install -y\
    fuse3-devel\
    squashfs\
    python3-pip \
    rpm-build
#   mpich\
#   mpich-devel
#   \
#   vim\
#   libmount-devel\
#   wget\
#   gzip\
#   autoconf\
#   automake\
#   libtool \
#   xz-devel\
#   liblz4-devel\
#   libzstd-devel\
#   lzo-devel\
#   zlib-devel\
#   htop\
#   nnn\
#   util-linux\
#   util-linux-systemd\
#   psmisc\
#   psutils\
#   strace \
#   git

# install meson and ninja via pip (packages are too old)
RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja

RUN mkdir /user-environment

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser


# RUN --mount=type=bind,source=../,target=/CODE \
#     CPPFLAGS=-I$SLURM_ROOT/include meson setup --prefix=$SLURM_ROOT /build /CODE \
#     && ninja install -C /build \
#     && echo "required $SLURM_ROOT/lib64/libslurm-uenv-mount.so" > $SLURM_ROOT/etc/plugstack.conf
