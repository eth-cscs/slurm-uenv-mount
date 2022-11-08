all: build install test

build:
	CPPFLAGS=-I$(SLURM_ROOT)/include meson setup --prefix=$(SLURM_ROOT) /build . 

install:
	ninja install -C /build 
	echo "required $(SLURM_ROOT)/lib64/libslurm-uenv-mount.so" > $(SLURM_ROOT)/etc/plugstack.conf

test:
	echo "Running tests..."
	echo "Running tests complete."

