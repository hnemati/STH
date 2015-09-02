
include ../target

SUBDIRS = minimal trusted # linux is build by make setup

all:
	 set -e; for d in $(SUBDIRS); do $(MAKE) -C $$d ; done

clean:
	 for d in $(SUBDIRS); do $(MAKE) clean -C $$d ; done

# setup build linux only if it is a target
setup:
ifneq (,$(findstring linux,$(SOFTWARE)))	
	make -C linux setup
endif
