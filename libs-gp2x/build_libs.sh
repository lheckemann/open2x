#!/bin/bash

# TODO: Add in all the lib build scripts and clean up the dependency logic a little. 
# (i.e. you can't even try and make SDL_image without SDL etc. etc.)

# $header$

# This is a script to build and install the GP2X patched libs found in the Open2x libs_gp2x CVS module.

# Tell the script where to find a suitable compiler

TOOLS_ROOT=/usr/local/devkitPro/devkitGP2X/arm-linux/bin
export TOOLS_ROOT

# Set this to the path you would like your libs installed under (e.g. include, libs etc. etc.)
# Default is setup to be compatible with DevKitGP2X's default install. (http://www.devkitpro.org/index.php)

LIB_ROOT=/usr/local/devkitPro/devkitGP2X
export LIB_ROOT

# 1 builds and installs the lib, 0 skips the install. 
# This is not clever and has not much of an idea about dependences so don't try and install SDL_image without SDL for example.

# General Limbs
BUILD_LIBJPEG=1
BUILD_LIBPNG=1
BUILD_LIBMAD=1
BUILD_LIBID3TAG=1
BUILD_FREETYPE=1
BUILD_LIBTREMOR=1

export BUILD_LIBJPEG BUILD_LIBPNG BUILD_LIBMAD BUILD_LIBID3TAG BUILD_FREETYPE BUILD_LIBTREMOR

# SDL Libs
BUILD_SDL=1
BUILD_SDL_IMAGE=1
BUILD_SDL_MIXER=1
BUILD_SDL_TTF=1

export BUILD_SDL BUILD_SDL_IMAGE BUILD_SDL_MIXER SDL_TTF

# Start calling the build scripts in order.

exit