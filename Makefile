# -*- Makefile -*-

#SUBDIRS = simulation library core drivers
SUBDIRS = utils library simulation drivers guests test core
#SUBDIRS = library simulation guests core

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done


clean:
	for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done
	rm -rf bin

setup: target clean
	make -C simulation setup	

target:
	echo "# -*- Makefile -*-" > target
	echo "# Target configuration" >> target
	echo "#PLATFORM=ovp_integratorCP" >> target
	echo "PLATFORM=beagleboard" >> target
	echo "#PLATFORM=beaglebone" >> target
	echo "#SOFTWARE=minimal" >> target
	echo "SOFTWARE = trusted linux" >> target
	echo "#SOFTWARE = linux" >> target

# testing
test: all
	make -C test test

.PHONY: test

##
sim: all
	make -C core sim
