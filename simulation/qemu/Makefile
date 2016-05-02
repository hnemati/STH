
COMMIT=e571600c4a

CFLAGS += -Wno-error=deprecated-declarations
CFLAGS += `pkg-config --cflags --libs glib-2.0`

all: ../build/qemu-system-arm


../build/qemu-system-arm: qemu-linaro
	-make -C qemu-linaro clean
	cd qemu-linaro && ./configure --target-list=arm-softmmu --prefix=../../bin
	make -C qemu-linaro  CFLAGS="$(CFLAGS)" -j`nproc`
	cp qemu-linaro/arm-softmmu/qemu-system-arm ../build/qemu-system-arm

qemu-linaro:
	git clone git://git.linaro.org/qemu/qemu-linaro.git qemu-linaro
	git -C qemu-linaro checkout $(COMMIT)
	git -C qemu-linaro submodule update --init dtc

clean:
	rm -rf qemu-linaro
