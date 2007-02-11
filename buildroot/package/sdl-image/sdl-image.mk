#############################################################
#
# SDL
#
#############################################################
ifeq ($(BR2_PACKAGE_SDL_IMAGE_VERSION), "latest")
SDL_IMAGE_VERSION:=latest
endif

ifeq ($(BR2_PACKAGE_SDL_IMAGE_VERSION), "1.2.5")
SDL_IMAGE_VERSION:=1.2.5
endif

SDL_CAT:=$(ZCAT)
SDL_IMAGE_DIR:=$(BUILD_DIR)/SDL_image-$(SDL_IMAGE_VERSION)
include package/sdl-image/sdl-image-$(SDL_IMAGE_VERSION).makefile

SDL-image sdl-image: sdl zlib tiff jpeg libpng $(SDL_IMAGE_DIR)/.installed

sdl-image-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) CC=$(TARGET_CC) -C $(SDL_IMAGE_DIR) uninstall
	-$(MAKE) -C $(SDL_IMAGE_DIR) clean

sdl-image-dirclean:
	rm -rf $(SDL_IMAGE_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_SDL_IMAGE)),y)
TARGETS+=sdl-image
endif