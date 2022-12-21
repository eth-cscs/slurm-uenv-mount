# FROM simonpzh/suse-slurm:20.11.9
FROM ghcr.io/eth-cscs/slurm-container-20.11.9:latest

RUN zypper install -y\
    fuse3-devel\
    squashfs\
    python3-pip\
    mpich\
    mpich-devel\
    vim\
    libmount-devel\
    wget\
    git \
    gzip\
    unzip \
    rpm-build \
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
    strace

# install meson and ninja via pip (packages are too old)
RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja
RUN wget https://github.com/vasi/squashfuse/archive/refs/tags/0.1.105.tar.gz \
    && tar xf 0.1.105.tar.gz \
    && cd squashfuse-0.1.105 || exit 1 \
    && ./autogen.sh && ./configure --prefix=/usr/local && sudo make install

RUN --mount=type=bind,source=./,target=/CODE \
    meson setup --prefix=/usr /build /CODE &&\
    ninja install  -C /build  &&\
    echo "required /usr/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf

RUN mkdir /user-environment

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser

# USER testuser
COPY docker/testuser-entrypoint.sh .

ENTRYPOINT ["/testuser-entrypoint.sh"]
