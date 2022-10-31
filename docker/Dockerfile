FROM simonpzh/slurm-dkr2.suse

RUN zypper install -y python3-pip

RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja
RUN zypper install -y fuse3-devel squashfs wget gzip autoconf automake libtool \
    xz-devel liblz4-devel libzstd-devel lzo-devel zlib-devel htop nnn psmisc strace
RUN  wget https://github.com/vasi/squashfuse/archive/refs/tags/0.1.105.tar.gz && tar xf 0.1.105.tar.gz &&\
    cd squashfuse-0.1.105 || exit 1 &&\
    ./autogen.sh && ./configure --prefix=/usr/local && sudo make install

RUN --mount=type=bind,source=./,target=/CODE \
    meson setup /build /CODE &&\
    ninja install  -C /build  &&\
    echo "required /usr/local/lib64/libslurm-uenv-mount.so" > /usr/local/etc/plugstack.conf

RUN mkdir /user-environment

RUN useradd testuser

# USER testuser
COPY docker/testuser-entrypoint.sh .

ENTRYPOINT ./testuser-entrypoint.sh
