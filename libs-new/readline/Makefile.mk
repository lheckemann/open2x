ifeq ($(strip $(TARGET_READLINE)),y)
TARGETS+=o2x-readline
endif

#include readline/5.1.mk
include readline/5.2.mk