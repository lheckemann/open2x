
 BUILD  = $(shell uname -m)
 HOST   = gp2x
 TARGET = gp2x
 PREFIX = /usr/local/gp2xdev
 FLAGS  = CC='gp2x-gcc' CXX='gp2x-g++' CFLAGS='-I$(PREFIX)/include -I$(PREFIX)/include/libxml2' LDFLAGS='-L$(PREFIX)/lib'

 TARGETS += $(PREFIX)/lib/libSDL.a
 TARGETS += $(PREFIX)/lib/libSDL_gfx.a
 TARGETS += $(PREFIX)/lib/libSDL_image.a
 TARGETS += $(PREFIX)/lib/libSDL_mixer.a
 TARGETS += $(PREFIX)/lib/libSDL_svg.a
 TARGETS += $(PREFIX)/lib/libSDL_ttf.a
 TARGETS += $(PREFIX)/lib/libfreetype.a
 TARGETS += $(PREFIX)/lib/libid3tag.a
 TARGETS += $(PREFIX)/lib/libjpeg.a
 TARGETS += $(PREFIX)/lib/libmad.a
 TARGETS += $(PREFIX)/lib/libmikmod.a
 TARGETS += $(PREFIX)/lib/libogg.a
 TARGETS += $(PREFIX)/lib/libpng.a
 TARGETS += $(PREFIX)/lib/libvorbis.a
 TARGETS += $(PREFIX)/lib/libxml2.a
 TARGETS += $(PREFIX)/lib/libvorbisidec.a
 TARGETS += $(PREFIX)/lib/libz.a

 all: $(TARGETS)

 clean:
	rm -f $(TARGETS)

 $(PREFIX)/lib/libSDL.a:
	cd SDL-1.2.9 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --enable-pthreads --enable-pthreads-sem --enable-threads && make && make install && make clean

 $(PREFIX)/lib/libSDL_gfx.a: $(PREFIX)/lib/libSDL.a
	cd SDL_gfx-2.0.13 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) --disable-mmx && make && make install && make clean

 $(PREFIX)/lib/libSDL_image.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libjpeg.a $(PREFIX)/lib/libpng.a
	cd SDL_image-1.2.4 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) && make && make install && make clean

 $(PREFIX)/lib/libSDL_mixer.a: $(PREFIX)/lib/libSDL.a  $(PREFIX)/lib/libmikmod.a $(PREFIX)/lib/libvorbisidec.a
	cd SDL_mixer-1.2.6 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) --enable-music-mp3=no && make && make install && make clean

 $(PREFIX)/lib/libSDL_svg.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libxml2.a
	cd SDL_svg-1.1.9 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) && make && make install && make clean

 $(PREFIX)/lib/libSDL_ttf.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libfreetype.a
	cd SDL_ttf-2.0.7 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-freetype-prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) && make && make install && make clean

 $(PREFIX)/lib/libfreetype.a:
	cd freetype-2.1.10 && GNUMAKE=make ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 $(PREFIX)/lib/libid3tag.a:
	cd libid3tag-0.15.1b && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 $(PREFIX)/lib/libjpeg.a:
	cd libjpeg-0.6b && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install-lib && make install-headers && make clean

 $(PREFIX)/lib/libmad.a:
	cd libmad-0.15.1b && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --enable-speed && make && make install && make clean

 $(PREFIX)/lib/libmikmod.a:
	cd libmikmod-3.1.11 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --disable-esd && make && make install && make clean

 $(PREFIX)/lib/libogg.a:
	cd libogg-1.1.3 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 $(PREFIX)/lib/libpng.a: $(PREFIX)/lib/libz.a
	cd libpng-1.2.8 && make -f scripts/makefile.linux && make -f scripts/makefile.linux install && make -f scripts/makefile.linux clean

 $(PREFIX)/lib/libvorbis.a: $(PREFIX)/lib/libogg.a
	cd libvorbis-1.1.2 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-ogg=$(PREFIX) && make && make install && make clean

 $(PREFIX)/lib/libxml2.a:
	cd libxml2-2.6.23 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 $(PREFIX)/lib/libvorbisidec.a:
	cd tremor-0.0.0 && ./autogen.sh && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --enable-low-accuracy && make && make install && make clean

 $(PREFIX)/lib/libz.a:
	cd zlib-1.2.3 && $(FLAGS) ./configure --prefix=$(PREFIX) && make && make install && make clean
	cd zlib-1.2.3 && $(FLAGS) ./configure --prefix=$(PREFIX) --shared && make && make install && make clean
