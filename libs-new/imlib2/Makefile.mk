ifeq ($(strip $(TARGET_IMLIB2)),y)
TARGETS+=o2x-imlib2
endif

#include imlib2/1.2.2.mk
include imlib2/1.4.0.mk