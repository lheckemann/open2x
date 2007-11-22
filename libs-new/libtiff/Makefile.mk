ifeq ($(strip $(TARGET_LIBTIFF)),y)
TARGETS+=o2x-libtiff
endif

include libtiff/3.8.2.mk