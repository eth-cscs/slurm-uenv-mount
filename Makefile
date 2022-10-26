.PHONY: install install-suid clean

prefix = /usr/local
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib64

CXXFLAGS ?= -Os -Wall -Wpedantic -fPIC -std=c++17 -shared
LDFLAGS ?= -Wl,--allow-shlib-undefined -shared -fPIC


SLURM_UENV_MOUNT_VERSION := $(shell cat VERSION)
SLURM_UENV_MOUNT_LDFLAGS =-lmount

RPMBUILD ?= rpmbuild

all: libslurm-uenv-mount

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

plugin.o,mount.o: VERSION mount.hpp

libslurm-uenv-mount: plugin.o mount.o
		$(CXX) $^ $(LDFLAGS)  $(SLURM_UENV_MOUNT_LDFLAGS) -o $@.so

install: libslurm-uenv-mount
	mkdir -p $(DESTDIR)$(libdir)
	cp -p libslurm-uenv-mount.so $(DESTDIR)$(libdir)

rpm: plugin.cpp mount.cpp mount.hpp VERSION LICENSE Makefile
	./generate-rpm.sh -b $@
	$(RPMBUILD) -bs --define "_topdir $@" $@/SPECS/slurm-uenv-mount.spec

clean:
	rm -rf libslurm-uenv-mount.so *.o rpm
