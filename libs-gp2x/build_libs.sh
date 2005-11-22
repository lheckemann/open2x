#!/bin/sh

# TODO: Add in all the lib build scripts and clean up the dependency logic a little. 
# (i.e. you can't even try and make SDL_image without SDL etc. etc.)

# $header$

# This is a script to build and install the GP2X patched libs found in the Open2x libs_gp2x CVS module.

echo " "
echo "*  Configuring GP2X library build scripts"
echo " "

# Tell the script where to find a suitable compiler and its name

HOST=arm-linux
TOOLS_ROOT=/usr/local/devkitPro/devkitGP2X/arm-linux/bin
export TOOLS_ROOT HOST
export PATH=$TOOLS_ROOT:$PATH

echo "-   Using tools from $TOOLS_ROOT"

# Set this to the path you would like your libs installed under (e.g. include, libs etc. etc.)
# Default is setup to be compatible with DevKitGP2X's default install. (http://www.devkitpro.org/index.php)

LIB_ROOT=/usr/local/devkitPro/devkitGP2X
export LIB_ROOT
echo "-   Installing libraries to $LIB_ROOT"

# Export your CFLAGS.
export CFLAGS=-msoft-float
export CXXFLAGS=$CFLAGS $CXXFLAGS
export CPPFLAGS=$CFLAGS $CPPFLAGS

# 1 builds and installs the lib, 0 skips the install. 
# This is not clever and has not much of an idea about dependences so don't try and install SDL_image without SDL for example.

# General Limbs
BUILD_LIBJPEG=0
BUILD_LIBPNG=0
BUILD_LIBMAD=0
BUILD_LIBID3TAG=0
BUILD_FREETYPE=0
BUILD_LIBTREMOR=0

export BUILD_LIBJPEG BUILD_LIBPNG BUILD_LIBMAD BUILD_LIBID3TAG BUILD_FREETYPE BUILD_LIBTREMOR

# SDL Libs
BUILD_SDL=0
BUILD_SDL_IMAGE=0
BUILD_SDL_MIXER=0
BUILD_SDL_TTF=0

export BUILD_SDL BUILD_SDL_IMAGE BUILD_SDL_MIXER BUILD_SDL_TTF

# Start calling the build scripts in order.

echo " "
echo "*  Building libraries selected in build_libs.sh"
echo " "

BUILD_LIBS_DIR=./build_libs
cd $BUILD_LIBS_DIR

# Build up general libs
./build_libPNG.sh
./build_libJPEG.sh
./build_libMAD.sh
./build_libTremor.sh
./build_freetype.sh

# Build up SDL based libs
./build_SDL.sh
./build_SDL_image.sh
./build_SDL_mixer.sh
./build_SDL_ttf.sh

echo " "
echo "*  It looks like we are done, check the output to see if anything went wrong"
echo "   These scripts are a little rough, please submit patches to Open2x if you"
echo "   find problems."

exit
