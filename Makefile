# -*- Makefile -*-

#SUBDIRS = simulation library core drivers
#SUBDIRS = utils library simulation drivers guests test core
SUBDIRS = utils library drivers guests test core simulation
#SUBDIRS = library simulation guests core

all:
	cat ../sth-linux-jonas/arch/arm/boot/zImage ../sth-linux-jonas/arch/arm/boot/dts/am335x-bone.dtb > guests/linux/build/zImage.bin;set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done
	rm -rf bin
# testing
test: all
	make -C test test
.PHONY: test

##
sim: all
	make -C core sim
