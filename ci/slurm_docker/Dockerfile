ARG BASE_IMAGE
FROM $BASE_IMAGE

COPY . /src
WORKDIR /src
RUN ./ci/slurm_docker/install_and_test.sh
