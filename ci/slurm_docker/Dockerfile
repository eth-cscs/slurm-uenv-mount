FROM opensuse/leap:15.4

ARG SLURM_VERSION=23.02.7
ARG SLURM_ROOT=/usr
ARG SLURM_CONFDIR=/etc/slurm

ENV SLURM_VERSION   ${SLURM_VERSION}
ENV SLURM_ROOT      ${SLURM_ROOT}
ENV SLURM_CONFDIR   ${SLURM_CONFDIR}


RUN zypper install -y       \
    munge                   \
    munge-devel             \
    libnuma1                \
    libnuma-devel           \
    librrd8                 \
    readline-devel          \
    hwloc                   \
    hwloc-devel             \
    lz4                     \
    liblz4-devel            \
    libz1                   \
    zlib-devel              \
    freeipmi                \
    freeipmi-devel          \
    dbus-1                  \
    dbus-1-devel            \
    make                    \
    gcc                     \
    gcc-c++                 \
    curl                    \
    tar                     \
    bzip2                   \
    python3                 \
    vim                     \
    ca-certificates         \
    less                    \
    sudo

RUN zypper install -y       \
    lua53                   \
    lua53-devel             \
    libmount-devel

RUN useradd -M slurm

RUN mkdir -p /var/log/slurm
RUN mkdir -p /var/spool/slurmctld && chown slurm /var/spool/slurmctld && chmod u+rwx /var/spool/slurmctld
RUN mkdir -p /var/spool/slurmd    && chown slurm /var/spool/slurmd    && chmod u+rwx /var/spool/slurmd


COPY install_slurm.sh .

RUN  ./install_slurm.sh ${SLURM_VERSION} ${SLURM_ROOT} ${SLURM_CONFDIR} --enable-multiple-slurmd

RUN  mkdir -p       ${SLURM_CONFDIR}
COPY cgroup.conf    ${SLURM_CONFDIR}
COPY slurm.conf.in  ${SLURM_CONFDIR}

COPY entrypoint.sh .
ENTRYPOINT ["./entrypoint.sh"]
CMD ["bash"]

