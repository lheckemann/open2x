ifeq ($(strip $(TARGET_BZIP2)),y)
TARGETS+=o2x-bzip2
endif

#include bzip2/1.0.3.mk
#include bzip2/1.0.4.mk
include bzip2/1.0.5.mk