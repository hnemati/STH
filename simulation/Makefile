include ../target

DIRS = qemu
DIRS_OPTIONAL = ovp_integratorCP ovp_arm9 generic_a8


all:
	@echo Nothing to do for simulation after setup

clean:
	@echo Nothing to clean in simulation folder

test:
	@echo Nothing to test in the simulation folder	

setup: build
	 -for d in $(DIRS_OPTIONAL); do $(MAKE) -C $$d ; done
	 set -e; for d in $(DIRS); do $(MAKE) -C $$d ; done

build:
	mkdir build

