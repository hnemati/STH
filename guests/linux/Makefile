
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

setup: tools build/zImage.bin

# note that we rebuild if target has changed!
build/zImage.bin: build sth-linux-stable  ../../target
	-$(MAKE) -C sth-linux-stable clean
	$(MAKE) -C sth-linux-stable copy

sth-linux-stable:
	@echo Downloading Linux, this may take some time!
	git clone --depth=1 -b $(TAG) --single-branch git@bitbucket.org:sicssec/sth-linux-stable.git


	# fix compiler name
	-sed -i.bak s/arm-none-linux-gnueabi/arm-linux-gnueabi/g sth-linux-stable/Makefile.sth

tools:
#	arm-linux-gnueabi-gcc --version || sudo apt-get install gcc-arm-linux-gnueabi

	# linus' tree wont build with gcc 5
	arm-linux-gnueabi-gcc-4.9 --version || sudo apt-get install gcc-4.9-arm-linux-gnueabi
	arm-linux-gnueabi-gcc --version || sudo ln -s /usr/bin/arm-linux-gnueabi-gcc-4.9 /usr/bin/arm-linux-gnueabi-gcc

##
build:
	mkdir build

clean:
	rm -rf build

delete: clean
	rm -rf sth-linux-stable
