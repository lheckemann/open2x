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


## -- MAIN U-BOOT BUILD SCRIPT

## -- FUNCTIONS

abort() {
	echo
	echo "Please Note - The U-Boot build process has failed."
	echo "Error below:"
	echo
	echo $@
	echo
	exec false
}

## -- END FUNCTIONS

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
echo Open2x - GP2X bootloader 'U-Boot' build script.
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
rm -f $WORK_DIR/gp2xboot.img
rm -f $WORK_DIR/gp2xkernel.img

clear

make mrproper
if [ $? != 0 ]; then
	abort "Error: make mrproper failed."
fi

make mmsp2dtk_config
if [ $? != 0 ]; then
	abort "Error: make mmsp2dtk_config failed."
fi

make
if [ $? != 0 ]; then
	abort "Error: make failed."
fi

echo Building Bootloader Package.

echo Collecting files...

mkdir "gp2xboot-open2x-`date '+%Y-%m-%d'`" > /dev/null
mkdir "gp2xboot-open2x-`date '+%Y-%m-%d'`/boot-as-kernel" > /dev/null
mkdir "gp2xboot-open2x-`date '+%Y-%m-%d'`/boot-as-boot" > /dev/null

cp ./gp2xboot.img ./gp2xboot-open2x-`date '+%Y-%m-%d'`/boot-as-boot > /dev/null
cp ./gp2xkernel.img ./gp2xboot-open2x-`date '+%Y-%m-%d'`/boot-as-kernel > /dev/null
cp ./README.Open2x ./gp2xboot-open2x-`date '+%Y-%m-%d'`/ > /dev/null

echo Building ZIP bundle...
if [ -f /usr/bin/zip ]
	then
		rm ./"gp2xboot-open2x-`date '+%Y-%m-%d'`.zip"
		cd "gp2xboot-open2x-`date '+%Y-%m-%d'`"
		zip -r -9 "../gp2xboot-open2x-`date '+%Y-%m-%d'`.zip" *
		echo You should have a "gp2xboot-open2x-`date '+%Y-%m-%d'`.zip" with the Open2x bootloader.
		cd ..
		rm -R ./"gp2xboot-open2x-`date '+%Y-%m-%d'`"
	else  
		echo - /usr/bin/zip not found, ZIP bundle not created.
		echo All distribution files can be found in ./"gp2xboot-open2x-`date '+%Y-%m-%d'`"
		echo - Please use you preferred archive tool to bundle these files.
fi

echo Done - Please check build logs.
