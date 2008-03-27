ifeq ($(strip $(TARGET_LIBICONV)),y)
TARGETS+=o2x-libiconv
endif

#include libiconv/1.11.mk
include libiconv/1.12.mk
