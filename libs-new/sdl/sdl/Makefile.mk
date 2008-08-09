ifeq ($(strip $(TARGET_SDL)),y)
TARGETS+=o2x-sdl
endif

#include sdl/sdl/1.2.9.mk
#include sdl/sdl/1.2.11.mk
include sdl/sdl/1.2.13.mk
