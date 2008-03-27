ifeq ($(strip $(TARGET_SDL_GFX)),y)
TARGETS+=o2x-sdl_gfx
endif

#include sdl/sdl_gfx/2.0.13.mk
#include sdl/sdl_gfx/2.0.15.mk
include sdl/sdl_gfx/2.0.16.mk