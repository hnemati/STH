#!/bin/bash
if [[ "$1" == *"--debug"* ]]
then
qemu-system-arm -M beagle -m 512 -sd ../../../../sth_deps/beagle_sd_working.img -nographic -s -S
else
qemu-system-arm -M beagle -m 512 -sd ../../../../sth_deps/beagle_sd_working.img -nographic
fi
