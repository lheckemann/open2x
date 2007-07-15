#!/bin/sh
arm-open2x-linux-gcc -c `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --cflags` -o installer.o installer.c
arm-open2x-linux-gcc -static installer.o -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lSDL_image -lpng -ljpeg -ltiff -lz -lSDL -lpthread -lm -s -o installer