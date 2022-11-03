# FROM ghcr.io/eth-cscs/slurm-container-20.11.9:latest
FROM simonpzh/slurm-20.11.9

RUN zypper install -y\
    fuse3-devel\
    squashfs\
    python3-pip\
    mpich\
    mpich-devel\
    vim\
    libmount-devel\
    wget\
    gzip\
    autoconf\
    automake\
    libtool \
    xz-devel\
    liblz4-devel\
    libzstd-devel\
    lzo-devel\
    zlib-devel\
    htop\
    nnn\
    util-linux\
    util-linux-systemd\
    psmisc\
    psutils\
    strace \
    git

# install meson and ninja via pip (packages are too old)
RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja
RUN wget https://github.com/vasi/squashfuse/archive/refs/tags/0.1.105.tar.gz \
    && tar xf 0.1.105.tar.gz \
    && cd squashfuse-0.1.105 || exit 1 \
    && ./autogen.sh && ./configure --prefix=/usr/local && sudo make install

RUN --mount=type=bind,source=../,target=/CODE \
    CPPFLAGS=-I$SLURM_ROOT/include meson setup --prefix=$SLURM_ROOT /build /CODE \
    && ninja install -C /build \
    && echo "required $SLURM_ROOT/lib64/libslurm-uenv-mount.so" > $SLURM_ROOT/etc/plugstack.conf

RUN mkdir /user-environment

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser

# USER testuser
COPY docker/testuser-entrypoint.sh .

<<<<<<< HEAD
<<<<<<< variant A
#ENTRYPOINT ./testuser-entrypoint-jpc.sh
>>>>>>> variant B
ENTRYPOINT ./testuser-entrypoint-jpc.sh
======= end
=======
ENTRYPOINT "./testuser-entrypoint.sh"
>>>>>>> 9cefa74 (merge dockerfile and entrypoint)
