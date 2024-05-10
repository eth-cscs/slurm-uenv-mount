ARG DOCKER_CONTAINER=ghcr.io/eth-cscs/slurm-container-20.11.9:latest
FROM ${DOCKER_CONTAINER}

RUN zypper install -y\
    fuse3-devel\
    squashfs\
    vim\
    libmount-devel\
    wget\
    git \
    gzip\
    unzip \
    rpm-build \
    autoconf\
    libcap-progs \
    automake\
    libtool \
    xz-devel\
    liblz4-devel\
    libzstd-devel\
    lzo-devel\
    openssl\
    libopenssl-devel\
    zlib-devel\
    htop\
    nnn\
    util-linux\
    util-linux-systemd\
    psmisc\
    psutils\
    strace \
    sqlite3 \
    sqlite3-devel

# install python
RUN curl -O https://www.python.org/ftp/python/3.10.11/Python-3.10.11.tgz \
    && tar xzvf Python-3.10.11.tgz \
    && cd Python-3.10.11 \
    && ./configure \
    && make install -j \
    && cd ../ && rm -r Python-3.10.11
RUN zypper --non-interactive rm libopenssl-devel

# rpmbuild > /usr/lib/rpm/macros.d/macros.meson are missing here ...
RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja
RUN curl https://raw.githubusercontent.com/mesonbuild/meson/master/data/macros.meson -o /usr/lib/rpm/macros.d/macros.meson

# download bash-bats
RUN curl -L https://github.com/bats-core/bats-core/archive/refs/tags/v1.9.0.tar.gz | tar xz
RUN ln -s /bats-core-1.9.0/bin/bats /usr/bin/bats

RUN mkdir bats-helpers
RUN git clone --depth 1 https://github.com/bats-core/bats-assert.git bats-helpers/bats-assert
RUN git clone --depth 1 https://github.com/bats-core/bats-support.git bats-helpers/bats-support
ENV BATS_LIB_PATH /bats-helpers

# rpm build expects meson in /usr/bin/meson
RUN ln -s /usr/local/bin/meson /usr/bin/meson

RUN wget https://github.com/vasi/squashfuse/archive/refs/tags/0.1.105.tar.gz \
    && tar xf 0.1.105.tar.gz \
    && cd squashfuse-0.1.105 || exit 1 \
    && ./autogen.sh && ./configure --prefix=/usr/local && sudo make install

RUN --mount=type=bind,source=./,target=/slurm-uenv-mount \
    meson setup /slurm-uenv-mount uenv-build \
    && meson compile -C uenv-build \
    && meson install -C uenv-build \
    && echo "required /usr/local/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf

RUN mkdir /user-environment
RUN mkdir /user-profilers
RUN mkdir /user-tools

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser

RUN --mount=type=bind,source=./,target=/slurm-uenv-mount \
    /slurm-uenv-mount/ci/create-dummy-sqfs.sh

# USER testuser
COPY docker/testuser-entrypoint.sh .

ENTRYPOINT ["/testuser-entrypoint.sh"]
