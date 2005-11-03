#Fill out later, notes for now. WILL NOT WORK ;-)
#TODO: Path this lib to support Tremor over the floating point Vorbix libs.

cd ..
cd SDL_mixer

export CFLAGS=-I/tools/open2x_gcc/arm-open2x-linux/include CPPFLAGS=-I/tools/open2x_gcc/arm-open2x-linux/include \
export CXXFLAGS=-I/tools/open2x_gcc/arm-open2x-linux/include LDFLAGS=-L/tools/open2x_gcc/arm-open2x-linux/lib \

./configure --with-sdl-prefix=/opt/local/gp2x --prefix /tools/open2x_gcc/arm-open2x-linux \
--host arm-open2x-linux --target arm-open2x-linux --build $(buildenv) --disable-music-libmikmod --disable-smpeg-test \
--disable-smpeg --without-smpeg