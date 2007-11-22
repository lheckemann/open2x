ifeq ($(strip $(TARGET_LIBPNG)),y)
TARGETS+=o2x-libpng
endif

#include libpng/1.2.8.mk
#include libpng/1.2.15.mk
include libpng/1.2.23.mk