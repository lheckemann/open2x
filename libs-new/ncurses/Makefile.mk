ifeq ($(strip $(TARGET_NCURSES)),y)
TARGETS+=o2x-ncurses
endif

#include ncurses/5.5.mk
include ncurses/5.6.mk