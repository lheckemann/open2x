ifeq ($(strip $(TARGET_FLAC)),y)
TARGETS+=o2x-flac
endif

include flac/1.2.1.mk