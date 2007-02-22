#############################################################
#
# glibc
#
#############################################################
GLIBC_CAT:=$(ZCAT)
GLIBC_TARGET_BINARY:=lib/libc.so

$(TARGET_DIR)/$(GLIBC_TARGET_BINARY):
	# Install components now
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/ld-* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libanl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libBrokenLocale*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libcrypt*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libdl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libanl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libgcc_s*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libanl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libm*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libnsl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libnss*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libpcprofile*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libanl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libpthread*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libresolv*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libanl*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/librt*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libSegFault*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libstdc++*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libthread_db*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libutil*so* $(TARGET_DIR)/lib
	cp -f $(BR2_STAGING_DIR)/arm-open2x-linux/lib/libc*so* $(TARGET_DIR)/lib
	find $(TARGET_DIR)/lib -type f -exec $(STRIP) --strip-debug '{}' ';'
	find $(TARGET_DIR)/usr/lib -type f -exec $(STRIP) --strip-debug '{}' ';'
	find $(TARGET_DIR)/bin -type f -exec $(STRIP) --strip-all '{}' ';'
	find $(TARGET_DIR)/sbin -type f -exec $(STRIP) --strip-all '{}' ';'
	find $(TARGET_DIR)/usr/bin -type f -exec $(STRIP) --strip-all '{}' ';'
	find $(TARGET_DIR)/usr/sbin -type f -exec $(STRIP) --strip-all '{}' ';'

glibc: $(TARGET_DIR)/$(GLIBC_TARGET_BINARY)

glibc-clean:
	echo Won't remove glibc

clibc-dirclean:
	echo glibc has no directory to remove

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_GLIBC)),y)
TARGETS+=glibc
endif
