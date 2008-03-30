ifeq ($(strip $(TARGET_LIBEXPAT)),y)
TARGETS+=o2x-libexpat
endif

include libexpat/2.0.1.mk