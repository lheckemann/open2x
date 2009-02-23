
.EXPORT_ALL_VARIABLES:



USEBG          = no
X86FB          = no 
STANDALONE     = no
OUTLINE_BITMAP = yes
USE_1ST_FB     = no 




ifeq ($(STANDALONE),yes)
	ifeq ($(X86FB),yes)
		CROSS_COMPILE = 
	else
		CROSS_COMPILE = arm_sa_le-
	endif
	CC = $(CROSS_COMPILE)gcc
endif

ifeq ($(STANDALONE),yes)
	LIBNAME = fbdisp
else
	LIBNAME = libfbdisp.a
endif

ifneq ($(STANDALONE),yes)
include ../config.mak
endif

SRCS    = main.c \
fblin32.c  \
fblin24.c  \
fblin16.c  \
scr_fb.c  \
gfxfontload.c  \
gfxfontout.c \
gulim_96_10_eng.c \
gulim_96_10_han.c \
gfxfontext.c

ifeq ($(STANDALONE),yes)
# image display test
SRCS += sondisp.c 
endif

OBJS	= $(SRCS:.c=.o)

ifeq ($(STANDALONE),yes)
	CFLAGS  = -g -I. 
else
	CFLAGS  = $(OPTFLAGS)
endif

ifeq ($(USEBG),yes)
	CFLAGS += -DUSE_DISP_BG_COLOR
endif	

ifeq ($(X86FB),yes)
	CFLAGS += -DX86FB
endif

ifeq ($(STANDALONE),yes)
	CFLAGS += -DSTANDALONE
endif
	
ifeq ($(OUTLINE_BITMAP),yes)
	CFLAGS += -DOUTLINE_BITMAP
endif

ifeq ($(USE_1ST_FB),yes)
	CFLAGS += -DUSE_1ST_FB
endif

# hhsong modify for 320X240
#ifeq ($(CONFIG_MMSP2_DTK3),y)
#	CFLAGS += -DDTK3
#endif

#ifeq ($(CONFIG_MMSP2_DTK4),y)
#	CFLAGS += -DDTK4
#endif

#ifeq ($(CONFIG_MMSP2_WDMS),y)
	CFLAGS += -DWDMS
#endif


.SUFFIXES: .c .o

# .PHONY: all clean

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

ifeq ($(STANDALONE),yes)
$(LIBNAME):	$(OBJS)
	$(CC) -o $(LIBNAME) $(OBJS)
	cp -a $(LIBNAME) /root/mmsp2nfs/root
else
$(LIBNAME):	$(OBJS)
	$(AR) r $(LIBNAME) $(OBJS)
endif	


all:	$(LIBNAME)

clean:
	rm -f $(LIBNAME) *.o *.a *~

distclean:
	rm -f $(LIBNAME) *.o *.a *~ .depend

dep:    depend

depend:
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend


#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
