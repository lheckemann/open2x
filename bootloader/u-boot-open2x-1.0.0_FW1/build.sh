#!/bin/bash

export PATH=/tools/open2x_gcc/gcc-3.4.4-glibc-2.2.5/arm-open2x-linux/arm-open2x-linux/bin:$PATH
export PATH=/tools/open2x_gcc/gcc-3.4.4-glibc-2.2.5/arm-open2x-linux/bin:$PATH
export CXX=arm-open2x-linux-g++
export CC=arm-open2x-linux-gcc
export CXXFLAGS=-march=armv4t
export LDFLAGS=-static

echo Building U-Boot for GP2X.
make