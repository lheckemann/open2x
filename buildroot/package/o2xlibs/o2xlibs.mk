#############################################################
#
# Open2x libs
#
#############################################################

O2XLIBS_VERSION:=0.9.1
O2XLIBS_SOURCE:=o2xlibs-$(O2XLIBS_VERSION).tar.bz2

$(DL_DIR)/$(O2XLIBS_SOURCE):
	$(WGET) -P $(DL_DIR) $(O2X_REPO)/o2xlibs/$(O2XLIBS_SOURCE)

$(TARGET_DIR)/lib/libdl.so: $(DL_DIR)/$(O2XLIBS_SOURCE)
	echo "Installing libraries..."
	tar xfvj $(DL_DIR)/$(O2XLIBS_SOURCE) -C $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/ld*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libc*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libstdc++*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libm*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libstdc++*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libgcc*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libpthread*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libdl*.so* $(TARGET_DIR)/lib/

o2xlibs: $(TARGET_DIR)/lib/libdl.so
	echo "Installed libraries!"

o2xlibs-clean:
	

o2xlibs-dirclean:
	
#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_O2XLIBS)),y)
TARGETS+=o2xlibs
endif
