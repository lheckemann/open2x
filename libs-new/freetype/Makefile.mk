ifeq ($(strip $(TARGET_FREETYPE)),y)
TARGETS+=o2x-freetype
endif

#include freetype/2.1.10.mk
include freetype/2.3.5.mk