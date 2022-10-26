FROM simonpzh/slurm-dkr2.suse

RUN zypper install -y python3-pip

RUN python3 -m pip install --upgrade pip && python3 -m pip install meson ninja

# RUN --mount=type=bind,source=./,target=/CODE \
#     meson setup /build /CODE &&\
#     meson install -C /build | tee build.log
RUN --mount=type=bind,source=./,target=/CODE \
    cd CODE &&\
    make &&\
    make rpm
