ifeq ($(strip $(TARGET_LIBOGG)),y)
TARGETS+=o2x-libogg
endif

include libogg/1.1.3.mk