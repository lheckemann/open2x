ifeq ($(strip $(TARGET_LIBID3TAG)),y)
TARGETS+=o2x-libid3tag
endif

include libid3tag/0.15.1b.mk