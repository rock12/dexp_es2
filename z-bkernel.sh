#!/bin/bash

export ARCH=arm CROSS_COMPILE=./toolchain/linaro-4.9/bin/arm-linux-androideabi-
make dexp_es2_defconfig
make -j4 zImage 2<&1 | tee build.log
