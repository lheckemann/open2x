ifeq ($(strip $(TARGET_ZLIB)),y)
TARGETS+=o2x-zlib
endif

include zlib/1.2.3.mk