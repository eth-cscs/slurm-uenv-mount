ARG DOCKER_CONTAINER
FROM ${DOCKER_CONTAINER}

RUN zypper install -y\
    fuse3-devel\
    squashfs\
    python3-pip \
    rpm-build

# install meson and ninja via pip (packages are too old)
RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja

RUN mkdir /user-environment

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser
