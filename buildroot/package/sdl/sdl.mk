#############################################################
#
# SDL
#
#############################################################
ifeq ($(BR2_PACKAGE_SDL_VERSION), "latest")
SDL_VERSION:=latest
endif

SDL_CAT:=$(ZCAT)
SDL_DIR:=$(BUILD_DIR)/SDL-$(SDL_VERSION)
include package/sdl/sdl-$(SDL_VERSION).makefile

SDL sdl: $(SDL_DIR)/.installed

sdl-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) CC=$(TARGET_CC) -C $(SDL_DIR) uninstall
	-$(MAKE) -C $(SDL_DIR) clean

sdl-dirclean:
	rm -rf $(SDL_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_SDL)),y)
TARGETS+=sdl
endif