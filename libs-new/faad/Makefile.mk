ifeq ($(strip $(TARGET_FAAD)),y)
TARGETS+=o2x-faad
endif

include faad/2.6.1.mk