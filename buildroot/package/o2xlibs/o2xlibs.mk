#############################################################
#
# Open2x libs
#
#############################################################

o2xlibs: 
	echo "Installing libraries..."
	cp -f -P $(BR2_TOOLCHAIN_LIBPACK)/*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/ld*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libc*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libstdc++*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libm*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libstdc++*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libgcc*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libpthread*.so* $(TARGET_DIR)/lib/
	cp -f -P $(BR2_TOOLCHAIN_EXTERNAL_PATH)/arm-open2x-linux/lib/libdl*.so* $(TARGET_DIR)/lib/
	#$(BR2_TOOLCHAIN_EXTERNAL_PATH)/bin/arm-open2x-linux-strip --strip-debug $(TARGET_DIR)/lib/*

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
