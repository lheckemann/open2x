#############################################################
#
# devfsd
#
#############################################################

DEVFSD_VERSION:=1.3.25
DEVFSD_SOURCE:=devfsd-$(DEVFSD_VERSION)-o2x.tar.bz2

$(TARGET_DIR)/sbin/devfsd:
	echo "Installing devfsd..."
	$(WGET) -P $(DL_DIR) $(O2X_REPO)/$(DEVFSD_SOURCE)
	mkdir -p $(BUILD_DIR)/devfsd-$(DEVFSD_VERSION)
	$(BZCAT) $(DL_DIR)/$(DEVFSD_SOURCE) | tar -C $(BUILD_DIR)/devfsd-$(DEVFSD_VERSION) $(TAR_OPTIONS) -
	cp -f $(BUILD_DIR)/devfsd-$(DEVFSD_VERSION)/devfsd $(TARGET_DIR)/sbin/

devfsd: $(TARGET_DIR)/sbin/devfsd
	echo "Installed devfsd..."

devfsd-clean:

devfsd-dirclean:
	
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_DEVFSD)),y)
TARGETS+=devfsd
endif
