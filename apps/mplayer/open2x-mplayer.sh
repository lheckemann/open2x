#!/bin/sh

## -- OPEN2X USER SETTINGS

## OPEN2X - This should point to the root of your tool-chain (i.e. folder above the BIN dir) 
 
OPEN2X=/opt/open2x/gcc-4.1.1-glibc-2.3.6
 
## CROSS_COMPILE - This should be the canonical tool names of your tool.
## Defaults would be 'arm-open2x-linux-' for a normal Open2x tool-chain.
 
CROSS_COMPILE=arm-open2x-linux-

AR = $CROSS_COMPILEar
CC = $CROSS_COMPILEgcc
STRIP = $CROSS_COMPILEstrip
HOST_CC = $CROSS_COMPILEgcc
BUILD_CC = gcc

## -- END OPEN2X USER SETTINGS

export OPEN2X
export CROSS_COMPILE
export AR
export CC
export STRIP
export HOST_CC
export BUILD_CC

PREFIX=$OPEN2X
export PREFIX
 
PATH=$PATH:$OPEN2X/bin
WORK_DIR=`pwd`

export PATH
export WORK_DIR

clear

echo
echo Open2x - GP2X mplayer build script.
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

clear

make

echo Done - Please check build logs.
