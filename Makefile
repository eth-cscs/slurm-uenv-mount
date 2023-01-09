.PHONY: all rpm install clean

prefix = /usr
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib64

CXXFLAGS ?= -Os -Wall -Wpedantic -fPIC -std=c++17 -shared
LDFLAGS ?= -Wl,--allow-shlib-undefined -shared -fPIC

FILES:=plugin.cpp mount.cpp mount.hpp VERSION LICENSE Makefile
CXXFILES:=$(filter %.cpp, $(FILES))
OBJFILES:=$(CXXFILES:%.cpp=%.o)

MAKEFILEDIR=$(dir $(firstword $(MAKEFILE_LIST)))
ifneq ($(CURDIR), $(realpath $(MAKEFILEDIR)))
	SRCDIR:=$(MAKEFILEDIR)
	FILES:=$(FILES:%=$(SRCDIR)%)
endif

SLURM_UENV_MOUNT_VERSION := $(shell sed 's/-/~/' $(SRCDIR)VERSION)
SLURM_UENV_MOUNT_LDFLAGS =-lmount

RPMPKG=slurm-uenv-mount-${SLURM_UENV_MOUNT_VERSION}
RPMBUILDDIR=rpm
RPMBUILD ?= rpmbuild

all: libslurm-uenv-mount.so

%.o: $(SRCDIR)%.cpp $(SRCDIR)VERSION $(SRCDIR)mount.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

libslurm-uenv-mount.so: $(OBJFILES)
		$(CXX) $^ $(LDFLAGS) $(SLURM_UENV_MOUNT_LDFLAGS) -o $@

install: libslurm-uenv-mount.so
	mkdir -p $(DESTDIR)$(libdir)
	cp -p libslurm-uenv-mount.so $(DESTDIR)$(libdir)

rpm: $(FILES) $(SRCDIR)slurm-uenv-mount.spec
	$(SRCDIR)./generate-rpm.sh --build $(RPMBUILDDIR) --src "$(SRCDIR)" --pkgname $(RPMPKG) --spec "$(SRCDIR)slurm-uenv-mount.spec"
	sed -i "s|UENVMNT_VERSION|$(SLURM_UENV_MOUNT_VERSION)|g" "$(RPMBUILDDIR)/SPECS/slurm-uenv-mount.spec"
	$(RPMBUILD) -bs --define "_topdir $(RPMBUILDDIR)" "$(RPMBUILDDIR)/SPECS/slurm-uenv-mount.spec"

clean:
	rm -rf libslurm-uenv-mount.so *.o rpm

