#############################################################
#
# SDL
#
#############################################################
SDL_CAT:=$(ZCAT)
SDL_GFX_DIR:=$(BUILD_DIR)/SDL_gfx-2.0.13
include package/sdl-gfx/sdl-gfx-2.0.13.makefile

SDL-gfx sdl-gfx: sdl $(SDL_GFX_DIR)/.installed

sdl-gfx-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) CC=$(TARGET_CC) -C $(SDL_GFX_DIR) uninstall
	-$(MAKE) -C $(SDL_GFX_DIR) clean

sdl-gfx-dirclean:
	rm -rf $(SDL_GFX_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_SDL_GFX)),y)
TARGETS+=sdl-gfx
endif