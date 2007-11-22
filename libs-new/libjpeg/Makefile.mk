ifeq ($(strip $(TARGET_LIBJPEG)),y)
TARGETS+=o2x-libjpeg
endif

include libjpeg/0.6b.mk