#############################################################
#
# joe
#
#############################################################
JOE_SOURCE:=joe-3.5.tar.gz
JOE_SITE:=http://mesh.dl.sourceforge.net/sourceforge/joe-editor/
JOE_CAT:=$(ZCAT)
JOE_DIR:=$(BUILD_DIR)/joe-3.5
JOE_BINARY:=joe
JOE_TARGET_BINARY:=bin/joe

$(DL_DIR)/$(JOE_SOURCE):
	 $(WGET) -P $(DL_DIR) $(JOE_SITE)/$(JOE_SOURCE)

$(DL_DIR)/$(JOE_PATCH):
	 $(WGET) -P $(DL_DIR) $(JOE_SITE)/$(JOE_PATCH)

joe-source: $(DL_DIR)/$(JOE_SOURCE) $(DL_DIR)/$(JOE_PATCH)

joe-unpacked: $(JOE_DIR)/.unpacked
$(JOE_DIR)/.unpacked: $(DL_DIR)/$(JOE_SOURCE) $(DL_DIR)/$(JOE_PATCH)
	$(JOE_CAT) $(DL_DIR)/$(JOE_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(JOE_DIR)/.unpacked

$(JOE_DIR)/.configured: $(JOE_DIR)/.unpacked
	(cd $(JOE_DIR); rm -rf config.cache; \
		CFLAGS=$(BR2_TARGET_CFLAGS) \
		./configure \
		--target=$(BR2_GNU_TARGET) \
		--host=$(BR2_GNU_TARGET) \
		--build=`uname -m` \
		--prefix=/usr \
	);
	touch $(JOE_DIR)/.configured

$(JOE_DIR)/$(JOE_BINARY): $(JOE_DIR)/.configured
	$(MAKE) -C $(JOE_DIR)

$(TARGET_DIR)/$(JOE_TARGET_BINARY): $(JOE_DIR)/$(JOE_BINARY)
	cp -a $(JOE_DIR)/$(JOE_BINARY) $(TARGET_DIR)/$(JOE_TARGET_BINARY)

joe: $(TARGET_DIR)/$(JOE_TARGET_BINARY)

joe-clean:
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(JOE_DIR) uninstall
	-$(MAKE) -C $(JOE_DIR) clean

joe-dirclean:
	rm -rf $(JOE_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_JOE)),y)
TARGETS+=joe
endif
