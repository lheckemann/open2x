ifeq ($(strip $(TARGET_LIBMPCDEC)),y)
TARGETS+=o2x-libmpcdec
endif

include libmpcdec/1.2.6.mk