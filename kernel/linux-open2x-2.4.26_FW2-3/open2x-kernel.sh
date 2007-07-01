#!/bin/sh

## -- OPEN2X USER SETTINGS

## OPEN2X - This should point to the root of your tool-chain (i.e. folder above the BIN dir) 
 
OPEN2X=/opt/open2x/gcc-3.4.4-glibc-2.3.6
 
## CROSS_COMPILE - This should be the canonical tool names of your tool.
## Defaults would be 'arm-open2x-linux-' for a normal Open2x tool-chain.
 
CROSS_COMPILE=arm-open2x-linux-

## U_BOOT_MKIMAGE - This is the location of your host OS specific version of MKIMAGE from U-Boot.
U_BOOT_MKIMAGE=/storage/file-store/Projects/GP2X/open2x-svn/bootloader/u-boot-open2x-1.0.0_FW2-3/tools

## -- END OPEN2X USER SETTINGS

export OPEN2X
export CROSS_COMPILE
export U_BOOT_MKIMAGE

PREFIX=$OPEN2X
export PREFIX
 
PATH=$PATH:$OPEN2X/bin
WORK_DIR=`pwd`


export PATH
export WORK_DIR

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

make clean
rm -f $WORK_DIR/zImage.gz
rm -f $WORK_DIR/gp2xkernel.img
clear

make menuconfig
make dep
make modules
make bzImage
gzip -c $WORK_DIR/arch/arm/boot/zImage > $WORK_DIR/zImage.gz
$U_BOOT_MKIMAGE/mkimage -A arm -O linux -T kernel -C gzip -a 00008000 -e 00008000 -n "Open2x Linux Kernel" -d $WORK_DIR/zImage.gz $WORK_DIR/gp2xkernel.img
make modules_install INSTALL_MOD_PATH=$WORK_DIR/modules

echo Done - Please check build logs.
