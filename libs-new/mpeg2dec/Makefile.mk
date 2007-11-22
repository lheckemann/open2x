ifeq ($(strip $(TARGET_MPEG2DEC)),y)
TARGETS+=o2x-mpeg2dec
endif

include mpeg2dec/gp2x.mk