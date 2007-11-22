ifeq ($(strip $(TARGET_LIBVORBIS)),y)
TARGETS+=o2x-libvorbis
endif

#include libvorbis/1.1.2.mk
include libvorbis/1.2.0.mk