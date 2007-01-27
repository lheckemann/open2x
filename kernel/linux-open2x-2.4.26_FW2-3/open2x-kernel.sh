#!/bin/sh

## -- OPEN2X USER SETTINGS

## OPEN2X - This should point to the root of your tool-chain (i.e. folder above the BIN dir) 
 
OPEN2X=/opt/open2x/gcc-3.4.4-glibc-2.3.6
 
## HOST and TARGET - These should be the canonical tool names of your tool.
## For the sake of this script HOST and TARGET should be the same.
## Defaults would be 'arm-open2x-linux' for a normal Open2x tool-chain.
 
HOST=arm-open2x-linux
TARGET=arm-open2x-linux
CROSS_COMPILE=arm-open2x-linux-

## -- END OPEN2X USER SETTINGS

export OPEN2X
export HOST
export TARGET
export CROSS_COMPILE

PREFIX=$OPEN2X
export PREFIX
 
PATH=$PATH:$OPEN2X/bin

export PATH

clear

echo
echo Open2x - GP2X kernel build script.
echo Please ensure you have configured this script to point to your Open2x GCC tool-chain
echo and approperate folder locations before you continue.
echo
echo Current settings.
echo 
echo Tool locations 		= $OPEN2X/bin
echo Cross Compiler		= $CROSS_COMPILE
echo 
read -p "Press return to continue." first
echo

make menuconfig
make dep
make modules
make bzImage
## gzip /arch/arm/boot/zImage ./zImage.gz


echo Done - Please check build logs.
