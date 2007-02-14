#!/bin/sh
# set -ex
RESULT_TOP=/opt/open2x
TARBALLS_DIR=/opt/open2x/downloads
export TARBALLS_DIR RESULT_TOP
GCC_LANGUAGES="c,c++"
export GCC_LANGUAGES

clear
echo
echo Open2x - "GPH Style" OLD Kernal Toolchain for Dynamic Linking on the GP2X
echo Based on GCC 2.9.5, gLibC 2.2.5
echo http://www.open2x.org/
echo
echo This it the old compatibility tool-chain, use it only if you are sure you 
echo need it otherwise use the newer 3.4.4 tool-chain for kernal development.
echo
echo Scripts based on Crosstool - http://kegel.com/crosstool/
echo
echo Do not run as ROOT - Repeat - Do not run as ROOT!
echo 
echo Tool-chain will be installed under $RESULT_TOP
echo Source archives will be placed in $TARBALLS_DIR
echo Please ensure your user has permissions to the above folders.
echo
read -p "Press any key to continue (or CTRL+C to exit)." first

# Really, you should do the mkdir before running this,
# and chown /opt/crosstool to yourself so you don't need to run as root
mkdir -p $RESULT_TOP.
mkdir -p $TARBALLS_DIR

# Build the toolchain.  Takes a couple hours and a couple gigabytes.

eval `cat arm-gp2x-kernel.dat gcc-2.95.3-glibc-2.2.5-hdrs-gp2x.dat` sh all.sh --notest

echo Done - Please check the logs to see if your build was a sucess.
