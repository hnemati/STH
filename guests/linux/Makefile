
TAG = STH_2.6

#

include ../../target

# XXX: some PLATFORM translation since they don't always match up
ifeq ($(PLATFORM),beagleboard)
	PLATFORM=beagleboard-xm
endif

#

MAKE = make -f Makefile.sth TARGET=$(PLATFORM) STH=../../..

#
all: build/zImage.bin

setup: build/zImage.bin

# note that we rebuild if target has changed!
build/zImage.bin: build sth-linux-stable  ../../target
	-cd sth-linux-stable && $(MAKE) clean
	cd sth-linux-stable && $(MAKE) copy

sth-linux-stable:
	@echo Downloading Linux, this may take some time!
	git clone --depth=1 -b $(TAG) --single-branch git@bitbucket.org:sicssec/sth-linux-stable.git

build:
	mkdir build

clean:
	rm -rf build

delete: clean
	rm -rf sth-linux-stable
	
