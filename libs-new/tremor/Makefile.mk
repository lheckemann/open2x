ifeq ($(strip $(TARGET_TREMOR)),y)
TARGETS+=o2x-tremor
endif

#include tremor/0.0.0.mk
include tremor/19102007.mk