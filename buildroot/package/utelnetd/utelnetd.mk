#############################################################
#
# GMenu2X
#
#############################################################

UTELNETD_VERSION:=0.1.9
UTELNETD_SOURCE:=utelnetd-$(UTELNETD_VERSION)-o2x.tar.bz2
O2X_REPO:=http://x11.gp2x.de/open2x/bins

utelnetd: 
	echo "Installing utelnetd..."
	$(WGET) -P $(DL_DIR) $(O2X_REPO)/$(UTELNETD_SOURCE)
	mkdir -p $(BUILD_DIR)/utelnetd-0.1.9
	$(BZCAT) $(DL_DIR)/$(UTELNETD_SOURCE) | tar -C $(BUILD_DIR)/utelnetd-0.1.9 $(TAR_OPTIONS) -
	cp -f $(BUILD_DIR)/utelnetd-0.1.9/utelnetd $(TARGET_DIR)/sbin/

utelnetd-clean:
	

utelnetd-dirclean:
	
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_UTELNETD)),y)
TARGETS+=utelnetd
endif
