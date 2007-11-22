ifeq ($(strip $(TARGET_SMPEG)),y)
TARGETS+=o2x-smpeg
endif

#include smpeg/0.4.4.mk
include smpeg/cvs.mk