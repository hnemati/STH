

all: ../build/qemu-system-arm


../build/qemu-system-arm: qemu-linaro
	-cd qemu-linaro && make clean
	cd qemu-linaro && ./configure --target-list=arm-softmmu --prefix=../../bin
	cd qemu-linaro && make -j8
	cp qemu-linaro/arm-softmmu/qemu-system-arm ../build/qemu-system-arm

qemu-linaro:
	git clone git://git.linaro.org/qemu/qemu-linaro.git qemu-linaro
	cd qemu-linaro && git checkout e571600c4a260d3072abbe7ecf2013e48e82bea7
