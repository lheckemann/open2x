
 BUILD  = $(shell uname -m)
 HOST   = open2x
 TARGET = open2x
 PREFIX = $(shell pwd)/temp

 FLAGS += CC='open2x-gcc'
 FLAGS += CXX='open2x-g++'
 FLAGS += CFLAGS='-I$(OPEN2X)/include -I$(OPEN2X)/include/libxml2'
 FLAGS += LDFLAGS='-L$(OPEN2X)/lib -R$(OPEN2X)/lib'

 all: busybox libs lua user-libs # python

 clean:
	rm -Rf $(PREFIX)

 busybox:
	cd busybox-1.1.1 && make CROSS=$(HOST)- && make PREFIX=$(PREFIX) install && make clean

 libs:
	mkdir -p $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/ld-linux.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libanl.so.1 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libBrokenLocale.so.1 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libcrypt.so.1 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libc.so.6 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libdl.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libm.so.6 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnsl.so.1 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnss_compat.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnss_dns.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnss_files.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnss_hesiod.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnss_nisplus.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libnss_nis.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libpthread.so.0 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libresolv.so.2 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/librt.so.1 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libthread_db.so.1 $(PREFIX)/lib
	cp $(OPEN2X)/open2x/lib/libutil.so.1 $(PREFIX)/lib
	chmod a-x $(PREFIX)/lib/*.so*

 user-libs:
	mkdir -p $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libfreetype.so.6 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libhistory.so.5 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libid3tag.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libjpeg.so.62 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libmad.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libmikmod.so.2 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libogg.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libpng12.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libpng.so.3 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libreadline.so.5 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libSDL-1.2.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libSDL_gfx.so.13 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libSDL_image-1.2.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libSDL_mixer-1.2.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libSDL_svg-1.1.so.8 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libSDL_ttf-2.0.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libsmpeg-0.4.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libtiff.so.3 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libtiffxx.so.3 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libungif.so.4 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libvorbisenc.so.2 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libvorbisfile.so.3 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libvorbisidec.so.1 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libvorbis.so.0 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libxml2.so.2 $(PREFIX)/usr/lib
	cp $(OPEN2X)/lib/libz.so.1 $(PREFIX)/usr/lib
	chmod a-x $(PREFIX)/usr/lib/*.so.*

 lua:
	cd lua-5.1 && make && make PREFIX=$(PREFIX) install && make clean

 python:
	mkdir -p python-2.4.3/build && cd python-2.4.3/build && $(FLAGS) ../configure --prefix=$(PWD)/$(PREFIX)/usr --build=$(BUILD) --host=$(HOST) --with-shared --with-static && make && make install && make clean
