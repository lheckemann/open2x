ifeq ($(strip $(TARGET_SDL_SVG)),y)
TARGETS+=o2x-sdl_svg
endif

include sdl/sdl_svg/1.1.9.mk