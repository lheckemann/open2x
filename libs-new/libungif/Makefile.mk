ifeq ($(strip $(TARGET_LIBUNGIF)),y)
TARGETS+=o2x-libungif
endif

include libungif/4.1.4.mk