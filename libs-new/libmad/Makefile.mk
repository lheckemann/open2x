ifeq ($(strip $(TARGET_LIBMAD)),y)
TARGETS+=o2x-libmad
endif

include libmad/0.15.1b.mk