ifeq ($(strip $(TARGET_LIBXML2)),y)
TARGETS+=o2x-libxml2
endif

#include libxml2/2.6.23.mk
#include libxml2/2.6.29.mk
#include libxml2/2.6.30.mk
include libxml2/2.6.31.mk