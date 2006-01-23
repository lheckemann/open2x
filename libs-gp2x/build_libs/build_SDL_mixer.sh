#!/bin/sh

# TODO: Add in all the lib build scripts and clean up the dependency logic a little. 
# (i.e. you can't even try and make SDL_image without SDL etc. etc.)

# $header$

# This is a script to build and install the GP2X patched SDL_mixer lib found in the Open2x libs_gp2x CVS module.

echo " "
echo "*  Building SDL_mixer 1.2.6 for the GP2X using OGG Vorbis (No MIDI/MP3 Support)"
echo " "

if [ $BUILD_SDL_MIXER = 1 ]
then

  echo "-   Using tools from $TOOLS_ROOT"
  echo "-   Installing libraries to $LIB_ROOT"
  #./configure --prefix=$LIB_ROOT --build=i386 --host=$HOST --target=$HOST --enable-video-fbcon --disable-video-qtopia --disable-video-photon --disable-video-direct --disable-video-ggi --disable-video-svga --disable-video-aalib --disable-video-dummy --disable-video-dga --disable-arts --disable-esd --disable-alsa --disable-video-x11 --disable-nas --disable-nasm --disable-debug --disable-joystick-amigaos --disable-joystick-beos --disable-joystick-bsd --disable-joystick-darwin --disable-joystick-dc --disable-joystick-linux --disable-joystick-macos --disable-joystick-mint --disable-joystick-win32 --disable-shared || exit
  #make || exit
  #make install || exit
  #cp /opt/local/gp2x/bin/arm-open2x-linux-sdl-config /opt/local/gp2x/bin/sdl-config
  echo "-   Build complete - returning to main script"

else

  echo "-   This library is not being built. BUILD_SDL_MIXER = $BUILD_SDL_MIXER"
  exit

fi

exit

#Fill out later, notes for now. WILL NOT WORK ;-)
#TODO: Path this lib to support Tremor over the floating point Vorbix libs.

#cd ..
#cd SDL_mixer

#export CFLAGS=-I/tools/open2x_gcc/arm-open2x-linux/include CPPFLAGS=-I/tools/open2x_gcc/arm-open2x-linux/include \
#export CXXFLAGS=-I/tools/open2x_gcc/arm-open2x-linux/include LDFLAGS=-L/tools/open2x_gcc/arm-open2x-linux/lib \

#./configure --with-sdl-prefix=/opt/local/gp2x --prefix /tools/open2x_gcc/arm-open2x-linux \
#--host arm-open2x-linux --target arm-open2x-linux --build $(buildenv) --disable-music-libmikmod --disable-smpeg-test \
#--disable-smpeg --without-smpeg