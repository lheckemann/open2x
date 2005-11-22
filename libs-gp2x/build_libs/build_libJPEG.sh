#!/bin/sh

# TODO: Add in all the lib build scripts and clean up the dependency logic a little. 
# (i.e. you can't even try and make SDL_image without SDL etc. etc.)

# $header$

# This is a script to build and install the GP2X patched JPEG viewing lib found in the Open2x libs_gp2x CVS module.

echo " "
echo "*  Building libJPEG for the GP2X"
echo " "

if [ $BUILD_LIBJPEG = 1 ]
then

  echo "-   Using tools from $TOOLS_ROOT"
  echo "-   Installing libraries to $LIB_ROOT"
  #./configure --prefix=$LIB_ROOT --build=i386 --host=$HOST --target=$HOST --enable-video-fbcon --disable-video-qtopia --disable-video-photon --disable-video-direct --disable-video-ggi --disable-video-svga --disable-video-aalib --disable-video-dummy --disable-video-dga --disable-arts --disable-esd --disable-alsa --disable-video-x11 --disable-nas --disable-nasm --disable-debug --disable-joystick-amigaos --disable-joystick-beos --disable-joystick-bsd --disable-joystick-darwin --disable-joystick-dc --disable-joystick-linux --disable-joystick-macos --disable-joystick-mint --disable-joystick-win32 --disable-shared || exit
  #make || exit
  #make install || exit
  #cp /opt/local/gp2x/bin/arm-open2x-linux-sdl-config /opt/local/gp2x/bin/sdl-config
  echo "-   Build complete - returning to main script"

else

  echo "-   This library is not being built. BUILD_LIBJPEG = $BUILD_LIBJPEG"
  exit

fi

exit
