include ../config.mak

LIBNAME = libmpcodecs.a

AUDIO_SRCS_NAT=ad_alaw.c ad_dk3adpcm.c ad_pcm.c ad_dvdpcm.c ad_imaadpcm.c ad_msadpcm.c ad_msgsm.c ad_ra1428.c
AUDIO_SRCS_OPT=ad_acm.c ad_dshow.c ad_qtaudio.c ad_liba52.c ad_faad.c ad_libvorbis.c ad_libmad.c ad_realaud.c ad_libdv.c
AUDIO_SRCS=dec_audio.c ad.c $(AUDIO_SRCS_LIB) $(AUDIO_SRCS_NAT) $(AUDIO_SRCS_OPT)

NATIVE_SRCS=native/RTjpegN.c native/minilzo.c native/xa_gsm.c native/decode144.c native/decode288.c

SRCS=$(AUDIO_SRCS) $(VIDEO_SRCS) $(VFILTER_SRCS) $(NATIVE_SRCS) img_format.c
OBJS=$(SRCS:.c=.o)

SRCS2=$(ENCODER_SRCS)
OBJS2=$(SRCS2:.c=.o)

CFLAGS  = $(OPTFLAGS) -I. -Inative -I.. -I../libmpdemux -I../loader $(EXTRA_INC) $(X264_INC) -D_GNU_SOURCE $(SDL_INC)

.SUFFIXES: .c .o

# .PHONY: all clean

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

all:    $(LIBNAME) 

$(LIBNAME):     $(OBJS)
	$(AR) r $(LIBNAME) $(OBJS)
	$(RANLIB) $(LIBNAME)


clean:
	rm -f *.o *.a *~
	rm -f native/*.o native/*.a native/*~

distclean:
	rm -f Makefile.bak $(OBJS) $(OBJS2) $(LIBNAME) *~ .depend

dep:    depend

depend:
	$(CC) -MM $(CFLAGS) $(SRCS) 1>.depend

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif

