#############################################################
#
# gmp
#
#############################################################
GMP_VERSION:=4.2.1
GMP_SOURCE:=gmp-$(GMP_VERSION).tar.bz2
GMP_SITE:=http://ftp.sunet.se/pub/gnu/gmp/
GMP_CAT:=$(BZCAT)
GMP_DIR:=$(BUILD_DIR)/gmp-$(GMP_VERSION)
GMP_BINARY:=libgmp.a
GMP_LIBVERSION:=3.4.1

ifeq ($(BR2_ENDIAN),"BIG")
GMP_BE:=yes
else
GMP_BE:=no
endif

$(DL_DIR)/$(GMP_SOURCE):
	 $(WGET) -P $(DL_DIR) $(GMP_SITE)/$(GMP_SOURCE)

libgmp-source: $(DL_DIR)/$(GMP_SOURCE)

$(GMP_DIR)/.unpacked: $(DL_DIR)/$(GMP_SOURCE)
	$(GMP_CAT) $(DL_DIR)/$(GMP_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	toolchain/patch-kernel.sh $(GMP_DIR) package/gmp/ \*.patch
	$(CONFIG_UPDATE) $(GMP_DIR)
	touch $(GMP_DIR)/.unpacked

$(GMP_DIR)/.configured: $(GMP_DIR)/.unpacked
	(cd $(GMP_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		ac_cv_c_bigendian=$(GMP_BE) \
		./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libdir=/lib \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--includedir=/include \
		--mandir=/usr/man \
		--infodir=/usr/info \
		--enable-shared \
		$(DISABLE_NLS) \
	);
	touch $(GMP_DIR)/.configured

$(GMP_DIR)/.libs/$(GMP_BINARY): $(GMP_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(GMP_DIR)

$(STAGING_DIR)/lib/$(GMP_BINARY): $(GMP_DIR)/.libs/$(GMP_BINARY)
	$(MAKE) prefix=$(STAGING_DIR) \
	    exec_prefix=$(STAGING_DIR) \
	    bindir=$(STAGING_DIR)/bin \
	    sbindir=$(STAGING_DIR)/sbin \
	    libexecdir=$(STAGING_DIR)/bin \
	    datadir=$(STAGING_DIR)/share \
	    sysconfdir=$(STAGING_DIR)/etc \
	    sharedstatedir=$(STAGING_DIR)/com \
	    localstatedir=$(STAGING_DIR)/var \
	    libdir=$(STAGING_DIR)/lib \
	    includedir=$(STAGING_DIR)/include \
	    oldincludedir=$(STAGING_DIR)/include \
	    infodir=$(STAGING_DIR)/info \
	    mandir=$(STAGING_DIR)/man \
            -C $(GMP_DIR) install

$(TARGET_DIR)/lib/libgmp.so.$(GMP_LIBVERSION): $(STAGING_DIR)/lib/$(GMP_BINARY)
	cp -a $(STAGING_DIR)/lib/libgmp.so* $(STAGING_DIR)/lib/libgmp.a \
		 $(TARGET_DIR)/lib/
ifeq ($(BR2_PACKAGE_LIBGMP_HEADERS),y)
	test -d $(TARGET_DIR)/usr/include || mkdir -p $(TARGET_DIR)/usr/include
	cp -a $(STAGING_DIR)/include/gmp.h $(TARGET_DIR)/usr/include/
endif
	$(STRIP) --strip-unneeded $(TARGET_DIR)/lib/libgmp.so* \
		$(TARGET_DIR)/lib/libgmp.a

libgmp: uclibc $(TARGET_DIR)/lib/libgmp.so.$(GMP_LIBVERSION)
libgmp-stage: uclibc $(STAGING_DIR)/lib/$(GMP_BINARY)

libgmp-clean:
	rm -f $(TARGET_DIR)/lib/$(GMP_BINARY) $(TARGET_DIR)/lib/libgmp.so* \
		$(TARGET_DIR)/usr/include/gmp.h
	-$(MAKE) -C $(GMP_DIR) clean

libgmp-dirclean:
	rm -rf $(GMP_DIR)

GMP_DIR2:=$(TOOL_BUILD_DIR)/gmp-$(GMP_VERSION)
$(GMP_DIR2)/.configured: $(GMP_DIR)/.unpacked
	[ -d $(GMP_DIR2) ] || mkdir $(GMP_DIR2)
	(cd $(GMP_DIR2); \
		CC_FOR_BUILD="$(HOSTCC)" \
		CC="$(HOSTCC)" \
		CFLAGS="$(HOST_CFLAGS)" \
		$(GMP_DIR)/configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libdir=/lib \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--includedir=/usr/include \
		--mandir=/usr/man \
		--infodir=/usr/info \
		--enable-shared \
		--enable-static \
		$(DISABLE_NLS) \
	);
	touch $(GMP_DIR2)/.configured

$(GMP_DIR2)/.libs/$(GMP_BINARY): $(GMP_DIR2)/.configured
	$(MAKE) -C $(GMP_DIR2)

GMP_HOST_DIR:=$(TOOL_BUILD_DIR)/gmp
$(GMP_HOST_DIR)/lib/$(GMP_BINARY): $(GMP_DIR2)/.libs/$(GMP_BINARY)
	mkdir -p $(GMP_HOST_DIR)/lib $(GMP_HOST_DIR)/include
	cp -a $(GMP_DIR2)/.libs/libgmp.* $(GMP_HOST_DIR)/lib
	cp -a $(GMP_DIR2)/gmp.h $(GMP_HOST_DIR)/include

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(strip $(BR2_PACKAGE_LIBGMP)),y)
TARGETS+=libgmp
endif