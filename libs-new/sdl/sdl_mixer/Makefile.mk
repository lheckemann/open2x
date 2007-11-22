ifeq ($(strip $(TARGET_SDL_MIXER)),y)
TARGETS+=o2x-sdl_mixer
endif

#include sdl/sdl_mixer/1.2.6.mk
#include sdl/sdl_mixer/1.2.7.mk
include sdl/sdl_mixer/1.2.8.mk