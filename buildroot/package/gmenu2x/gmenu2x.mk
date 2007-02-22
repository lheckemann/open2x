#############################################################
#
# GMENU2X
#
#############################################################
GMENU2X_CAT:=$(ZCAT)
GMENU2X_DIR:=$(BUILD_DIR)/gmenu2x
include package/gmenu2x/gmenu2x-0.8.1.makefile

gmenu2x: sdl sdl-gfx sdl-image $(GMENU2X_DIR)/.installed

gmenu2x-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) CC=$(TARGET_CC) -C $(GMENU2X_DIR) uninstall
	-$(MAKE) -C $(GMENU2X_DIR) clean

gmenu2x-dirclean:
	rm -rf $(GMENU2X_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_GMENU2X)),y)
TARGETS+=gmenu2x
endif