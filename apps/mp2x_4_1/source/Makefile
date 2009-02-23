#/*
# Example Makefile
#*/

.EXPORT_ALL_VARIABLES:

include 	config.mak

INCLUDES	= -I. -I./fbdisp
#EXTRALIBS	= -L$(TCDIR)/lib -lSDL_image -lmad -lvorbisidec -lz -lnsl  -L$(TCDIR)/lib -Wl,-rpath,$(TCDIR)/lib -lSDL -lpthread  -ldl -lm  -lpng -lz -ljpeg -lm -lunicodefont -lSDL_inifile
EXTRALIBS	= -L$(TCDIR)/lib -lSDL_image -lmad -lvorbisidec -lvorbisfile -lz -lnsl  -L$(TCDIR)/lib -Wl,-rpath,$(TCDIR)/lib -lSDL -lpthread  -ldl -lm  -lpng -lz -ljpeg -lm -lunicodefont -lSDL_inifile -larmmem

SUBLIBS		= libmpcodecs/libmpcodecs.a libaf/libaf.a libmpdemux/libmpdemux.a libao2/libao2.a osdep/libosdep.a fbdisp/libfbdisp.a liba52/liba52.a libfaad2/libfaad2.a
LIBS		= $(SUBLIBS) $(EXTRALIBS)
CFLAGS		= $(OPTFLAGS) -Ilibmpdemux -Iloader $(FREETYPE_INC) $(EXTRA_INC) $(CDPARANOIA_INC) $(SDL_INC) $(X11_INC) $(DVB_INC) $(XVID_INC) $(FONTCONFIG_INC) # -Wall
LDFLAGS		= 

ifeq ($(STRIPBINARIES),yes)
LDFLAGS += -s
endif

INCLUDES	+= -Ilibmpdemux -Iloader $(EXTRA_INC) $(SDL_INC) 

TGT_NAME	= mp2x
HOMEPATH	= .
TGT_DIR		= /usr/include
PRG_CFG         = codec-cfg.exe
SUBDIR 		= libmpdemux libmpcodecs libao2 osdep libaf fbdisp liba52 libfaad2

VPATH	= \
.\

include csource.lst

OBJFILES	= $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(FILES)))

default		: $(PRG_CFG) $(TGT_NAME)

$(PRG_CFG): version.h codec-cfg.c codec-cfg.h
	$(HOST_CC) -g codec-cfg.c mp_msg.c -o $(PRG_CFG) -DCODECS2HTML
	./$(PRG_CFG) ./etc/codecs.conf > codecs.conf.h


$(TGT_NAME)	: $(OBJFILES) $(SUBLIBS)
		@echo "Making ./$(TGT_NAME) ..."
#		@echo "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJFILES) $(LIBS)"
		echo "$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJFILES) $(LIBS)"
#		@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJFILES) $(LIBS)
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJFILES) $(LIBS)

libaf/libaf.a:
	$(MAKE) -C libaf

libmpdemux/libmpdemux.a:
	$(MAKE) -C libmpdemux

libmpcodecs/libmpcodecs.a:
	$(MAKE) -C libmpcodecs

libao2/libao2.a:
	$(MAKE) -C libao2

liba52/liba52.a:
	$(MAKE) -C liba52

libfaad2/libfaad2.a:
	$(MAKE) -C libfaad2
	
fbdisp/libfbdisp.a:
	$(MAKE) -C fbdisp

osdep/libosdep.a:
	$(MAKE) -C osdep
	
libavcodec/libavcodec.a:
	$(MAKE) -C libavcodec LIBPREF=lib LIBSUF=.a

%o : %s
		@echo "Assembling $< ..."
		@ $(RM) $@
		@$(CC) $(CFLAGS_AS) -c $<

%o : %c
		@echo "Compiling $< ..."
		@$(RM) $@
#		@$(CC) $(CFLAGS) -c $<
		$(CC) $(CFLAGS) -c $<


clean		:
		@echo "Cleaning all generated files ..."
		@$(RM) *.o
		@$(RM) $(TGT_NAME) $(PRG_CFG) codecs.conf.h .depend
		@for a in $(SUBDIR); do $(MAKE) -C $$a distclean; done

depend		:
		@echo "Making dependency list ..."
		@$(RM) *.o
		$(CC) -MM $(INCLUDES) -DCODECS2HTML $(FILES) 1>.depend
		@for a in $(SUBDIR); do $(MAKE) -C $$a depend; done
		@$(RM) *.o
		
install :
	cp $(TGT_NAME) .. -f
	$(STRIP) ../$(TGT_NAME)

all		:
		@make clean
		@make depend
		@make
		@make install

#
# It is possible to generate dependency lists automatically by depending on
# a make include file:
#
#  depend.$(TGT_NAME) can make automatically just type
#
#  % make depend
#

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
