#############################################################
#
# bftpd
#
#############################################################

BFTPD_VERSION:=1.9.1
BFTPD_SOURCE:=bftpd/bftpd-$(BFTPD_VERSION)-o2x.tar.bz2

$(TARGET_DIR)/sbin/bftpd:
	echo "Installing bftpd..."
	$(WGET) -P $(DL_DIR) $(O2X_REPO)/$(BFTPD_SOURCE)
	mkdir -p $(BUILD_DIR)/bftpd-$(BFTPD_VERSION)
	$(BZCAT) $(DL_DIR)/$(BFTPD_SOURCE) | tar -C $(BUILD_DIR)/bftpd-$(BFTPD_VERSION) $(TAR_OPTIONS) -
	cp -f $(BUILD_DIR)/bftpd-$(BFTPD_VERSION)/bftpd $(TARGET_DIR)/sbin/
	cp -f $(BUILD_DIR)/bftpd-$(BFTPD_VERSION)/bftpd.conf $(TARGET_DIR)/etc/
	cp -f $(BUILD_DIR)/bftpd-$(BFTPD_VERSION)/ftpdpassword $(TARGET_DIR)/etc/
	cp -f $(BUILD_DIR)/bftpd-$(BFTPD_VERSION)/Obftpd $(TARGET_DIR)/etc/init.d/
	cp -f $(BUILD_DIR)/bftpd-$(BFTPD_VERSION)/FTPServer $(TARGET_DIR)/usr/menu/sections/settings/

bftpd: $(TARGET_DIR)/sbin/bftpd
	echo "Installed bftpd..."

bftpd-clean:
	

bftpd-dirclean:
	
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_BFTPD)),y)
TARGETS+=bftpd
endif
