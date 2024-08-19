ARG BASE_IMAGE
FROM $BASE_IMAGE

COPY . /src

RUN CXX=g++-12 CC=gcc-12 meson setup builddir /src \
    && meson install -C builddir \
    && echo "required /usr/local/lib64/libslurm-uenv-mount.so" > /etc/slurm/plugstack.conf
