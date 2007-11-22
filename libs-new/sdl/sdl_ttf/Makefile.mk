ifeq ($(strip $(TARGET_SDL_TTF)),y)
TARGETS+=o2x-sdl_ttf
endif

#include sdl/sdl_ttf/2.0.7.mk
#include sdl/sdl_ttf/2.0.8.mk
include sdl/sdl_ttf/2.0.9.mk