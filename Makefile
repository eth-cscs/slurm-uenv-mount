SLURM_ROOT?=/opt/slurm
PREFIX?=$(SLURM_ROOT)

#CXXFLAGS ?= -Os -Wall -Wpedantic -fPIC -std=c++17 -shared
#LDFLAGS ?= -Wl,--allow-shlib-undefined -shared -fPIC

BUILD_DIR?=build
RPM_BUILD_DIR?=$(BUILD_DIR)/rpm

SLURM_UENV_MOUNT_VERSION := $(shell sed s/-dev// VERSION)

RPMBUILD ?= rpmbuild

build:
	CPPFLAGS=-I$(SLURM_ROOT)/include meson setup --prefix=$(PREFIX) $(BUILD_DIR) .
	ninja -C $(BUILD_DIR)

install: 
	ninja install -C $(BUILD_DIR)
	mkdir -p $(PREFIX)/etc/
	echo "required $(SLURM_ROOT)/lib64/libslurm-uenv-mount.so" > $(PREFIX)/etc/plugstack.conf

test:
	@echo "Running tests..."
	@echo "Running tests complete."

rpm: plugin.cpp mount.cpp mount.hpp VERSION LICENSE Makefile meson.build
	# Strip `-dev` from VERSION if exsists because RPM requires version numbers of the form X.Y.Z
	sed 's/UENVMNT_VERSION/$(SLURM_UENV_MOUNT_VERSION)/g' slurm-uenv-mount.spec.in > /tmp/slurm-uenv-mount.spec
	./generate-rpm.sh   \
	    --src=.	    \
	    --files="$^"    \
	    --build=$(RPM_BUILD_DIR)	    \
	    --pkgname="slurm-uenv-mount-$(SLURM_UENV_MOUNT_VERSION)"\
	    --spec-in=/tmp/slurm-uenv-mount.spec
	rm /tmp/slurm-uenv-mount.spec
	$(RPMBUILD) -ba --define "_topdir $(RPM_BUILD_DIR)" $(RPM_BUILD_DIR)/SPECS/slurm-uenv-mount.spec
	#$(RPMBUILD) -bs --define "_topdir $(RPM_BUILD_DIR)" $(RPM_BUILD_DIR)/SPECS/slurm-uenv-mount.spec

rpm-build: build

rpm-install:
	ninja install -C $(BUILD_DIR)


install-from-rpm:
	rpm -ivh --replacepkgs --prefix=$(SLURM_ROOT) $(RPM_BUILD_DIR)/RPMS/$(shell uname -i)/slurm-uenv-mount-0.1-1.$(shell uname -i).rpm
			

all: build test rpm

clean:
	rm -rf build

.PHONY: install clean rpm
