
 BUILD   = $(shell uname -m)
 HOST    = open2x
 TARGET  = open2x
 PREFIX  = $(OPEN2X)

 CFLAGS  = -I$(PREFIX)/include -I$(PREFIX)/include/libxml2
 LDFLAGS = -L$(PREFIX)/lib -R$(PREFIX)/lib
 CC      = $(HOST)-gcc $(CFLAGS)
 CXX     = $(HOST)-g++ $(CFLAGS)
 FLAGS   = CC='$(CC)' CXX='$(CXX)' CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)'

 TARGETS += $(PREFIX)/bin/python
 TARGETS += $(PREFIX)/lib/libImlib2.a
 TARGETS += $(PREFIX)/lib/libSDL.a
 TARGETS += $(PREFIX)/lib/libSDL_gfx.a
 TARGETS += $(PREFIX)/lib/libSDL_image.a
 TARGETS += $(PREFIX)/lib/libSDL_mixer.a
 TARGETS += $(PREFIX)/lib/libSDL_svg.a
 TARGETS += $(PREFIX)/lib/libSDL_ttf.a
 TARGETS += $(PREFIX)/lib/libbz2.a
 TARGETS += $(PREFIX)/lib/libfreetype.a
 TARGETS += $(PREFIX)/lib/libid3tag.a
 TARGETS += $(PREFIX)/lib/libjpeg.a
 TARGETS += $(PREFIX)/lib/liblua.a
 TARGETS += $(PREFIX)/lib/libmad.a
 TARGETS += $(PREFIX)/lib/libmikmod.a
 TARGETS += $(PREFIX)/lib/libncurses.a
 TARGETS += $(PREFIX)/lib/libogg.a
 TARGETS += $(PREFIX)/lib/libpng.a
 TARGETS += $(PREFIX)/lib/libreadline.a
 TARGETS += $(PREFIX)/lib/libsmpeg.a
 TARGETS += $(PREFIX)/lib/libtiff.a
 TARGETS += $(PREFIX)/lib/libungif.a
 TARGETS += $(PREFIX)/lib/libvorbis.a
 TARGETS += $(PREFIX)/lib/libvorbisidec.a
 TARGETS += $(PREFIX)/lib/libxml2.a
 TARGETS += $(PREFIX)/lib/libz.a

 all: $(TARGETS)

 clean:
	rm -f $(TARGETS)

 SDL: $(PREFIX)/lib/libSDL.a
 $(PREFIX)/lib/libSDL.a:
	cd SDL-1.2.9 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --enable-pthreads --enable-pthreads-sem --enable-threads && make && make install && make clean

 SDL_gfx: $(PREFIX)/lib/libSDL_gfx.a
 $(PREFIX)/lib/libSDL_gfx.a: $(PREFIX)/lib/libSDL.a
	cd SDL_gfx-2.0.13 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) --disable-mmx && make && make install && make clean

 SDL_image: $(PREFIX)/lib/libSDL_image.a
 $(PREFIX)/lib/libSDL_image.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libjpeg.a $(PREFIX)/lib/libpng.a
	cd SDL_image-1.2.4 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) && make && make install && make clean

 SDL_mixer: $(PREFIX)/lib/libSDL_mixer.a
 $(PREFIX)/lib/libSDL_mixer.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libsmpeg.a $(PREFIX)/lib/libvorbisidec.a
	cd SDL_mixer-1.2.6 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) --enable-music-mp3=yes --enable-music-libmikmod=no && make && make install && make clean

 SDL_svg: $(PREFIX)/lib/libSDL_svg.a
 $(PREFIX)/lib/libSDL_svg.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libxml2.a
	cd SDL_svg-1.1.9 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) && make && make install && make clean

 SDL_ttf: $(PREFIX)/lib/libSDL_ttf.a
 $(PREFIX)/lib/libSDL_ttf.a: $(PREFIX)/lib/libSDL.a $(PREFIX)/lib/libfreetype.a
	cd SDL_ttf-2.0.7 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-freetype-prefix=$(PREFIX) --with-sdl-prefix=$(PREFIX) && make && make install && make clean

 bzip2: $(PREFIX)/lib/libbz2.a
 $(PREFIX)/lib/libbz2.a:
	cd bzip2-1.0.3 && make && make install && make clean

 freetype: $(PREFIX)/lib/libfreetype.a
 $(PREFIX)/lib/libfreetype.a:
	cd freetype-2.1.10 && GNUMAKE=make ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 imlib2: $(PREFIX)/lib/libImlib2.a
 $(PREFIX)/lib/libImlib2.a: $(PREFIX)/lib/libjpeg.a $(PREFIX)/lib/libpng.a $(PREFIX)/lib/libtiff.a $(PREFIX)/lib/libungif.a $(PREFIX)/lib/libz.a $(PREFIX)/lib/libbz2.a $(PREFIX)/lib/libid3tag.a $(PREFIX)/lib/libfreetype.a
	cd imlib2-1.2.2 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --disable-shared --enable-static --with-freetype-config=$(PREFIX)/bin/freetype-config --without-x && make && make install && make clean

 libid3tag: $(PREFIX)/lib/libid3tag.a
 $(PREFIX)/lib/libid3tag.a:
	cd libid3tag-0.15.1b && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 libjpeg: $(PREFIX)/lib/libjpeg.a
 $(PREFIX)/lib/libjpeg.a:
	cd libjpeg-0.6b && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install-lib && make install-headers && make clean

 libmad: $(PREFIX)/lib/libmad.a
 $(PREFIX)/lib/libmad.a:
	cd libmad-0.15.1b && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --enable-speed && make && make install && make clean

 libmikmod: $(PREFIX)/lib/libmikmod.a
 $(PREFIX)/lib/libmikmod.a:
	cd libmikmod-3.1.11 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --disable-esd && make && make install && make clean

 libogg: $(PREFIX)/lib/libogg.a
 $(PREFIX)/lib/libogg.a:
	cd libogg-1.1.3 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 libpng: $(PREFIX)/lib/libpng.a
 $(PREFIX)/lib/libpng.a: $(PREFIX)/lib/libz.a
	cd libpng-1.2.8 && $(FLAGS) make -f scripts/makefile.linux && $(FLAGS) make -f scripts/makefile.linux install && $(FLAGS) make -f scripts/makefile.linux clean

 libtiff: $(PREFIX)/lib/libtiff.a
 $(PREFIX)/lib/libtiff.a:
	cd libtiff-3.8.2 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 libungif: $(PREFIX)/lib/libungif.a
 $(PREFIX)/lib/libungif.a:
	cd libungif-4.1.4 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 libvorbis: $(PREFIX)/lib/libvorbis.a
 $(PREFIX)/lib/libvorbis.a: $(PREFIX)/lib/libogg.a
	cd libvorbis-1.1.2 && $(FLAGS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --with-ogg=$(PREFIX) && make && make install && make clean

 libxml2: $(PREFIX)/lib/libxml2.a
 $(PREFIX)/lib/libxml2.a:
	cd libxml2-2.6.23 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 lua: $(PREFIX)/lib/liblua.a
 $(PREFIX)/lib/liblua.a: $(PREFIX)/lib/libreadline.a $(PREFIX)/lib/libncurses.a
	cd lua-5.1 && make && make install && make clean

 ncurses: $(PREFIX)/lib/libncurses.a
 $(PREFIX)/lib/libncurses.a:
	cd ncurses-5.5 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 python: $(PREFIX)/bin/python
 $(PREFIX)/bin/python:
	mkdir -p python-2.4.3/build && cd python-2.4.3/build && $(FLAGS) ../configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --with-shared --with-static && make && make install && make clean

 readline: $(PREFIX)/lib/libreadline.a
 $(PREFIX)/lib/libreadline.a:
	cd readline-5.1 && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static && make && make install && make clean

 smpeg: $(PREFIX)/lib/libsmpeg.a
 $(PREFIX)/lib/libsmpeg.a:
	cd smpeg-0.4.4 && ./autogen.sh && $(LIBS) ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --disable-gtk-player --disable-opengl-player --enable-shared --enable-static --with-sdl-prefix=$(PREFIX) --without-x && make && make install && make clean

 tremor: $(PREFIX)/lib/libvorbisidec.a
 $(PREFIX)/lib/libvorbisidec.a:
	cd tremor-0.0.0 && ./autogen.sh && ./configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --enable-shared --enable-static --enable-low-accuracy && make && make install && make clean

 zlib: $(PREFIX)/lib/libz.a
 $(PREFIX)/lib/libz.a:
	cd zlib-1.2.3 && $(FLAGS) ./configure --prefix=$(PREFIX) && make && make install && make clean
	cd zlib-1.2.3 && $(FLAGS) ./configure --prefix=$(PREFIX) --shared && make && make install && make clean
