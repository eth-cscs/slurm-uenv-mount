FROM simonpzh/slurm-dkr2.suse

RUN zypper install -y python3-pip

RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja

# TODO: make meson find spank.h from non-standard location
RUN mkdir -p /usr/include/slurm
RUN ln -s /opt/slurm-20.11.9/include/slurm/spank.h /usr/include/slurm

RUN --mount=type=bind,source=./,target=/CODE \
    meson setup /build /CODE &&\
    meson install -C /build | tee build.log
