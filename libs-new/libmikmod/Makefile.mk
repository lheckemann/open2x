ifeq ($(strip $(TARGET_LIBMIKMOD)),y)
TARGETS+=o2x-libmikmod
endif

include libmikmod/3.1.11.mk