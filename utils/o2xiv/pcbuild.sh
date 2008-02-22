#!/bin/sh
gcc -lSDL_image -lSDL_gfx `sdl-config --cflags --libs` source/main.c -o out