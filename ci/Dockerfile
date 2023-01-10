ARG DOCKER_CONTAINER
FROM ${DOCKER_CONTAINER}

RUN zypper install -y\
    fuse3-devel\
    squashfs\
    rpm-build

RUN mkdir /user-environment

RUN useradd testuser
RUN mkdir -p /home/testuser
RUN chown testuser /home/testuser

COPY ci/tests /tests
RUN chown -R testuser /tests
