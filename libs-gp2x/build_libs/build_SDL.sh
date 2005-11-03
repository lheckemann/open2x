#!/bin/bash
#Borrowed from Snaff's quick SDL build script.

export PATH=/tools/open2x_gcc/gcc-4.0.2-glibc-2.3.5/arm-open2x-linux/arm-open2x-linux/bin:$PATH
export CFLAGS=-msoft-float

cp -r /gp2x/library/SDL-1.2.7/ .
cd SDL-1.2.7/

./configure --prefix=/opt/local/gp2x --build=i386 --host=arm-open2x-linux --target=arm-open2x-linux --enable-video-fbcon --disable-video-qtopia --disable-video-photon --disable-video-direct --disable-video-ggi --disable-video-svga --disable-video-aalib --disable-video-dummy --disable-video-dga --disable-arts --disable-esd --disable-alsa --disable-video-x11 --disable-nas --disable-nasm --disable-debug --disable-joystick-amigaos --disable-joystick-beos --disable-joystick-bsd --disable-joystick-darwin --disable-joystick-dc --disable-joystick-linux --disable-joystick-macos --disable-joystick-mint --disable-joystick-win32 --disable-shared || exit
make || exit
make install || exit

cp /opt/local/gp2x/bin/arm-open2x-linux-sdl-config /opt/local/gp2x/bin/sdl-config

cd ..
rm -rf SDL-1.2.7/

echo Build and installation complete
