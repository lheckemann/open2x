# Makefile to build and install the SDL_mixer library

top_builddir = .
srcdir  = .
objects = build
depend	= build-deps
prefix = /opt/open2x/gcc-4.1.1-glibc-2.3.6
exec_prefix = ${prefix}
bindir	= $(DESTDIR)${exec_prefix}/bin
libdir  = $(DESTDIR)${exec_prefix}/lib
includedir = $(DESTDIR)${prefix}/include
datadir	= $(DESTDIR)${prefix}/share
mandir	= $(DESTDIR)${prefix}/man
auxdir	= ./build-scripts
distpath = $(srcdir)/..
distdir = SDL_mixer-1.2.7
distfile = $(distdir).tar.gz


EXE	= 
SHELL	= /bin/bash
CC      = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-gcc
CFLAGS  = -O3 -ffast-math -fomit-frame-pointer -mcpu=arm920t -DARM -D_ARM_ASSEM_ -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/libxml2 -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/SDL 
EXTRA_CFLAGS =  -D_GNU_SOURCE=1 -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -DCMD_MUSIC -DWAV_MUSIC -DMOD_MUSIC -I$(srcdir)/mikmod -DMID_MUSIC -DUSE_TIMIDITY_MIDI -I$(srcdir)/timidity -DOGG_MUSIC -DOGG_DYNAMIC=\"libvorbisfile.so.3\" -DMP3_MUSIC -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/smpeg -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LDFLAGS = -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib
EXTRA_LDFLAGS =  -L/usr/local/lib -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lSDL -lpthread -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lsmpeg -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lSDL -lpthread
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INSTALL = /usr/bin/install -c
AR	= /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ar
RANLIB	= /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ranlib
SDL_CFLAGS = -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
SDL_LIBS = -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lSDL -lpthread

TARGET  = libSDL_mixer.la
SOURCES =  ./effect_position.c ./effect_stereoreverse.c ./effects_internal.c ./load_aiff.c ./load_voc.c ./mixer.c ./music.c ./music_cmd.c ./wavestream.c ./mikmod/*.c ./timidity/*.c ./*_ogg.c ./*_mp3.c
OBJECTS = $(objects)/effect_position.lo $(objects)/effect_stereoreverse.lo $(objects)/effects_internal.lo $(objects)/load_aiff.lo $(objects)/load_voc.lo $(objects)/mixer.lo $(objects)/music.lo $(objects)/music_cmd.lo $(objects)/wavestream.lo $(objects)/drv_nos.lo $(objects)/load_it.lo $(objects)/load_mod.lo $(objects)/load_s3m.lo $(objects)/load_xm.lo $(objects)/mdreg.lo $(objects)/mdriver.lo $(objects)/mloader.lo $(objects)/mlreg.lo $(objects)/mlutil.lo $(objects)/mmalloc.lo $(objects)/mmerror.lo $(objects)/mmio.lo $(objects)/mplayer.lo $(objects)/munitrk.lo $(objects)/mwav.lo $(objects)/npertab.lo $(objects)/sloader.lo $(objects)/virtch.lo $(objects)/virtch2.lo $(objects)/virtch_common.lo $(objects)/common.lo $(objects)/ctrlmode.lo $(objects)/filter.lo $(objects)/instrum.lo $(objects)/mix.lo $(objects)/output.lo $(objects)/playmidi.lo $(objects)/readmidi.lo $(objects)/resample.lo $(objects)/sdl_a.lo $(objects)/sdl_c.lo $(objects)/tables.lo $(objects)/timidity.lo $(objects)/dynamic_ogg.lo $(objects)/load_ogg.lo $(objects)/music_ogg.lo $(objects)/dynamic_mp3.lo

DIST = CHANGES COPYING CWProjects.sea.bin MPWmake.sea.bin Makefile.in README SDL_mixer.h SDL_mixer.qpg.in SDL_mixer.spec SDL_mixer.spec.in VisualC.zip Watcom-OS2.zip Xcode.tar.gz acinclude.m4 autogen.sh build-scripts configure configure.in dynamic_mp3.c dynamic_mp3.h dynamic_ogg.c dynamic_ogg.h effect_position.c effect_stereoreverse.c effects_internal.c effects_internal.h gcc-fat.sh load_aiff.c load_aiff.h load_ogg.c load_ogg.h load_voc.c load_voc.h mikmod mixer.c music.c music_cmd.c music_cmd.h music_ogg.c music_ogg.h native_midi native_midi_gpl playmus.c playwave.c timidity wavestream.c wavestream.h

LT_AGE      = 2
LT_CURRENT  = 2
LT_RELEASE  = 1.2
LT_REVISION = 5
LT_LDFLAGS  = -no-undefined -rpath $(libdir) -release $(LT_RELEASE) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

all: $(srcdir)/configure Makefile $(objects) $(objects)/$(TARGET) $(objects)/playwave$(EXE) $(objects)/playmus$(EXE)

$(srcdir)/configure: $(srcdir)/configure.in
	@echo "Warning, configure.in is out of date"
	#(cd $(srcdir) && sh autogen.sh && sh configure)
	@sleep 3

Makefile: $(srcdir)/Makefile.in
	$(SHELL) config.status $@

$(objects):
	$(SHELL) $(auxdir)/mkinstalldirs $@

$(objects)/playwave$(EXE): $(objects)/playwave.lo $(objects)/$(TARGET)
	$(LIBTOOL) --mode=link $(CC) -o $@ $(objects)/playwave.lo $(SDL_CFLAGS) $(SDL_LIBS) $(objects)/$(TARGET)

$(objects)/playmus$(EXE): $(objects)/playmus.lo $(objects)/$(TARGET)
	$(LIBTOOL) --mode=link $(CC) -o $@ $(objects)/playmus.lo $(SDL_CFLAGS) $(SDL_LIBS) $(objects)/$(TARGET)

.PHONY: all depend install install-hdrs install-lib install-bin uninstall uninstall-hdrs uninstall-lib uninstall-bin clean distclean dist
depend:
	@SOURCES="$(SOURCES)" INCLUDE="$(INCLUDE)" output="$(depend)" \
	$(SHELL) $(auxdir)/makedep.sh
	echo "\$$(objects)/playwave.lo: \$$(srcdir)/playwave.c" >>$(depend)
	echo "	\$$(LIBTOOL) --mode=compile \$$(CC) \$$(CFLAGS) \$$(SDL_CFLAGS) -c \$$(srcdir)/playwave.c  -o \$$@" >>$(depend)
	echo "" >>$(depend)
	echo "\$$(objects)/playmus.lo: \$$(srcdir)/playmus.c" >>$(depend)
	echo "	\$$(LIBTOOL) --mode=compile \$$(CC) \$$(CFLAGS) \$$(SDL_CFLAGS) -c \$$(srcdir)/playmus.c  -o \$$@" >>$(depend)
	echo "" >>$(depend)

include $(depend)

$(objects)/$(TARGET): $(OBJECTS)
	$(LIBTOOL) --mode=link $(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LT_LDFLAGS)

install: all install-hdrs install-lib #install-bin
install-hdrs:
	$(SHELL) $(auxdir)/mkinstalldirs $(includedir)/SDL
	for src in $(srcdir)/SDL_mixer.h; do \
	    file=`echo $$src | sed -e 's|^.*/||'`; \
	    $(INSTALL) -m 644 $$src $(includedir)/SDL/$$file; \
	done
install-lib:
	$(SHELL) $(auxdir)/mkinstalldirs $(libdir)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(TARGET) $(libdir)/$(TARGET)
install-bin:
	$(SHELL) $(auxdir)/mkinstalldirs $(bindir)
	$(LIBTOOL) --mode=install $(INSTALL) -m 755 $(objects)/playwave$(EXE) $(bindir)/playwave$(EXE)
	$(LIBTOOL) --mode=install $(INSTALL) -m 755 $(objects)/playmus$(EXE) $(bindir)/playmus$(EXE)

uninstall: uninstall-hdrs uninstall-lib uninstall-bin
uninstall-hdrs:
	for src in $(srcdir)/SDL_mixer.h; do \
	    file=`echo $$src | sed -e 's|^.*/||'`; \
	    rm -f $(includedir)/SDL/$$file; \
	done
	-rmdir $(includedir)/SDL
uninstall-lib:
	$(LIBTOOL) --mode=uninstall rm -f $(libdir)/$(TARGET)
uninstall-bin:
	rm -f $(bindir)/playwave$(EXE)
	rm -f $(bindir)/playmus$(EXE)

clean:
	rm -rf $(objects)

distclean: clean
	rm -f Makefile
	rm -f SDL_mixer.qpg
	rm -f config.status config.cache config.log libtool $(depend)
	rm -rf $(srcdir)/autom4te*
	find $(srcdir) \( \
	    -name '*~' -o \
	    -name '*.bak' -o \
	    -name '*.old' -o \
	    -name '*.rej' -o \
	    -name '*.orig' -o \
	    -name '.#*' \) \
	    -exec rm -f {} \;

dist $(distfile):
	$(SHELL) $(auxdir)/mkinstalldirs $(distdir)
	tar cf - $(DIST) | (cd $(distdir); tar xf -)
	rm -rf `find $(distdir) -name .svn`
	rm -f `find $(distdir) -name '.#*'`
	tar cvf - $(distdir) | gzip --best >$(distfile)
	rm -rf $(distdir)

rpm: $(distfile)
	rpmbuild -ta $?
