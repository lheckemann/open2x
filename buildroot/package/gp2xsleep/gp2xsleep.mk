#############################################################
#
# gp2xsleep
#
#############################################################

GP2XSLEEP_VERSION:=0.1
GP2XSLEEP_SOURCE:=gp2xsleep/gp2xsleep-$(GP2XSLEEP_VERSION)-o2x.tar.bz2

$(TARGET_DIR)/usr/bin/gp2xsleep:
	echo "Installing gp2xsleep..."
	$(WGET) -P $(DL_DIR) $(O2X_REPO)/$(GP2XSLEEP_SOURCE)
	$(BZCAT) $(DL_DIR)/$(GP2XSLEEP_SOURCE) | tar -C $(TARGET_DIR)/usr/bin/ $(TAR_OPTIONS) -

gp2xsleep: $(TARGET_DIR)/usr/bin/gp2xsleep
	echo "Installed gp2xsleep..."

gp2xsleep-clean:
	

gp2xsleep-dirclean:
	
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_GP2XSLEEP)),y)
TARGETS+=gp2xsleep
endif
