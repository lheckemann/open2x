#!/bin/sh
gcc -Wall -O -I/usr/include -o png2linuxlogo-static png2linuxlogo.c /usr/lib/libpng.a /usr/lib/libz.a -lm
