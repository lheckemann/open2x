#!/bin/sh

## -- OPEN2X USER SETTINGS

## OPEN2X - This should point to the root of your tool-chain (i.e. folder above the BIN dir) 
 
OPEN2X=/opt/open2x/gcc-4.1.1-glibc-2.3.6
 
## HOST and TARGET - These should be the canonical tool names of your tool.
## For the sake of this script HOST and TARGET should be the same.
## Defaults would be 'arm-open2x-linux' for a normal Open2x tool-chain.
 
HOST=arm-open2x-linux
TARGET=arm-open2x-linux

## -- END OPEN2X USER SETTINGS

export OPEN2X
export HOST
export TARGET

PREFIX=$OPEN2X
export PREFIX
 
PATH=$PATH:$OPEN2X/bin

export PATH

clear

echo
echo Open2x - GP2X patched development libs. Build and Install script.
echo Please ensure you have configured this script to point to your Open2x GCC tool-chain.
echo and approperate folder locations before you continue.
echo
echo Current settings.
echo 
echo Install root/Working dir	= $OPEN2X
echo Tool locations 		= $OPEN2X/bin
echo Host/Target		= $HOST / $TARGET
echo Building			= Everything or just $1
echo 
read -p "Press return to continue." first
echo

make -f makefile.o2x $1 $2

echo Done - Please check build logs.
