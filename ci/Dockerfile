ARG DOCKER_CONTAINER
FROM ${DOCKER_CONTAINER}

RUN zypper install -y\
    fuse3-devel\
    git \
    libopenssl-devel\
    util-linux\
    util-linux-systemd\
    squashfs\
    rpm-build

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
# rpm build expects meson in /usr/bin/meson
RUN ln -s /usr/local/bin/meson /usr/bin/meson

# download bash-bats
RUN curl -L https://github.com/bats-core/bats-core/archive/refs/tags/v1.9.0.tar.gz | tar xz
RUN ln -s /bats-core-1.9.0/bin/bats /usr/bin/bats
RUN mkdir bats-helpers
RUN git clone --depth 1 https://github.com/bats-core/bats-assert.git bats-helpers/bats-assert
RUN git clone --depth 1 https://github.com/bats-core/bats-support.git bats-helpers/bats-support
ENV BATS_LIB_PATH /bats-helpers

RUN mkdir /user-environment
RUN mkdir /user-profilers
RUN mkdir /user-tools

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser

COPY ci/tests /tests
RUN chown -R testuser /tests
