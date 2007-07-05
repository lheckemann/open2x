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


## -- MAIN KERNEL BUILD SCRIPT

## -- FUNCTIONS

abort() {
	echo
	echo "Please Note - The Linux Kernel build process has failed."
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
if [ $? != 0 ]; then
	abort "Error: make menuconfig failed."
fi

make dep
if [ $? != 0 ]; then
	abort "Error: make dep failed."
fi

make bzImage
if [ $? != 0 ]; then
	abort "Error: make bzImage failed."
fi

make modules
if [ $? != 0 ]; then
	abort "Error: make modules failed."
fi

make modules_install INSTALL_MOD_PATH=$WORK_DIR/modules
if [ $? != 0 ]; then
	abort "Error: make modules_install failed."
fi

gzip -c $WORK_DIR/arch/arm/boot/zImage > $WORK_DIR/zImage.gz
if [ $? != 0 ]; then
	abort "Error: Could not create GZipped Kernel image."
fi

if [ -f $U_BOOT_MKIMAGE/mkimage ]
	then
		$U_BOOT_MKIMAGE/mkimage -A arm -O linux -T kernel -C gzip -a 00008000 -e 00008000 -n "Open2x Linux Kernel" -d $WORK_DIR/zImage.gz $WORK_DIR/gp2xkernel.img
		if [ $? != 0 ]; then
			abort "Error: U-Boot mkimage failed. You will NOT have a GP2XKERNEL.IMG!"
		fi
	else
		abort "Error: U-Boot mkimage not found (Need to install?). You will NOT have a GP2XKERNEL.IMG!"
fi

echo Building Kernel Package.

echo Collecting files...

mkdir "gp2xkernel-open2x-`date '+%Y-%m-%d'`" > /dev/null
mkdir "gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules" > /dev/null
mkdir "gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules/lib" > /dev/null
mkdir "gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules/lib/modules" > /dev/null
mkdir "gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules/lib/modules/2.4.26-open2x" > /dev/null

echo "#!/bin/bash" >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "echo Installing Modules..." >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "cp -R -f ./modules/lib/modules/2.4.26-open2x /lib/modules/2.4.26-open2x" >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "# Sync the SD card to check that everything is written." >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "sync" >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "# Return to the GPH menu screen" >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "cd /usr/gp2x" >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"
echo "exec /usr/gp2x/gp2xmenu" >> "gp2xkernel-open2x-`date '+%Y-%m-%d'`/inst_modules.gpe"

cp ./gp2xkernel.img ./gp2xkernel-open2x-`date '+%Y-%m-%d'`/ > /dev/null
cp ./README.Open2x ./gp2xkernel-open2x-`date '+%Y-%m-%d'`/ > /dev/null
cp -R ./modules/lib/modules/2.4.26-open2x/kernel ./gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules/lib/modules/2.4.26-open2x/kernel  > /dev/null
cp -R ./modules/lib/modules/2.4.26-open2x/pcmcia ./gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules/lib/modules/2.4.26-open2x/pcmcia  > /dev/null
cp ./modules/lib/modules/2.4.26-open2x/modules.dep ./gp2xkernel-open2x-`date '+%Y-%m-%d'`/modules/lib/modules/2.4.26-open2x/modules.dep > /dev/null

echo Building ZIP bundle...
if [ -f /usr/bin/zip ]
	then
		rm ./"gp2xkernel-open2x-`date '+%Y-%m-%d'`.zip"
		cd "gp2xkernel-open2x-`date '+%Y-%m-%d'`"
		zip -r -9 "../gp2xkernel-open2x-`date '+%Y-%m-%d'`.zip" *
		echo You should have a "gp2xkernel-open2x-`date '+%Y-%m-%d'`.zip" with the Open2x kernel and your modules.
		cd ..
		rm -R ./"gp2xkernel-open2x-`date '+%Y-%m-%d'`"
	else  
		echo - /usr/bin/zip not found, ZIP bundle not created.
		echo All included files can also be found in ./"gp2xkernel-open2x-`date '+%Y-%m-%d'`"
		echo - Please use you preferred archive tool to bundle these files.
fi

echo Done - Please check build logs.
