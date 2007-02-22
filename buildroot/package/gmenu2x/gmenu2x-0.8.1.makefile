############################################
#
# gmenu2x
#
############################################

$(GMENU2X_DIR)/.unpacked:
	# Download and prepare the source
	$(WGET) -P $(DL_DIR) http://heanet.dl.sourceforge.net/sourceforge/gmenu2x/gmenu2x-0.8.1.tar.bz2
	$(BZCAT) $(DL_DIR)/gmenu2x-0.8.1.tar.bz2 | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	toolchain/patch-kernel.sh $(GMENU2X_DIR) package/gmenu2x gmenu2x*.patch
	touch $(GMENU2X_DIR)/.unpacked

$(GMENU2X_DIR)/.configured: $(GMENU2X_DIR)/.unpacked
	touch $(GMENU2X_DIR)/.configured

$(GMENU2X_DIR)/.compiled: $(GMENU2X_DIR)/.configured
	$(MAKE) -C $(GMENU2X_DIR)/build
	touch $(GMENU2X_DIR)/.compiled

$(GMENU2X_DIR)/.installed: $(GMENU2X_DIR)/.compiled
	mkdir -p $(TARGET_DIR)/opt/gmenu2x
	cp -dpf $(GMENU2X_DIR)/build/gmenu2x $(TARGET_DIR)/opt/gmenu2x
	cp -dpf $(GMENU2X_DIR)/build/gmenu2x.conf $(TARGET_DIR)/opt/gmenu2x
	cp -dpfr $(GMENU2X_DIR)/build/icons $(TARGET_DIR)/opt/gmenu2x
	cp -dpfr $(GMENU2X_DIR)/build/imgs $(TARGET_DIR)/opt/gmenu2x
	cp -dpfr $(GMENU2X_DIR)/build/scripts $(TARGET_DIR)/opt/gmenu2x
	cp -dpfr $(GMENU2X_DIR)/build/sections $(TARGET_DIR)/opt/gmenu2x
	touch $(GMENU2X_DIR)/.installed