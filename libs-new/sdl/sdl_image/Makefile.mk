ifeq ($(strip $(TARGET_SDL_IMAGE)),y)
TARGETS+=o2x-sdl_image
endif

#include sdl/sdl_image/1.2.4.mk
include sdl/sdl_image/1.2.6.mk