############################################
#
# SDL
#      Download and compile the latest HW 
#      SDL code from the SVN.
#
############################################

SDL_SITE:=http://www.libsdl.org/projects/SDL_image/release/

$(SDL_IMAGE_DIR)/.unpacked:
	# Download and prepare the source
	$(WGET) -P $(DL_DIR) http://www.libsdl.org/projects/SDL_image/release/SDL_image-1.2.5.tar.gz
	$(ZCAT) $(DL_DIR)/SDL_image-1.2.5.tar.gz | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(SDL_IMAGE_DIR)/.unpacked

$(SDL_IMAGE_DIR)/.configured: $(SDL_IMAGE_DIR)/.unpacked
	(cd $(SDL_IMAGE_DIR); \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)" \
	./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=$(STAGING_DIR)/usr \
		--with-sdl-prefix=$(STAGING_DIR) \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libdir=$(STAGING_DIR)/lib \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--includedir=/include \
		--mandir=/usr/man \
		--infodir=/usr/info \
		--enable-shared \
		--enable-static \
		--without-iconv );
	touch $(SDL_IMAGE_DIR)/.configured

$(SDL_IMAGE_DIR)/.compiled: $(SDL_IMAGE_DIR)/.configured
	LDFLAGS="-L$(STAGING_DIR)/lib -lSDL" $(MAKE) -C $(SDL_IMAGE_DIR)
	touch $(SDL_IMAGE_DIR)/.compiled

$(SDL_IMAGE_DIR)/.installed: $(SDL_IMAGE_DIR)/.compiled
	$(MAKE) DESTDIR=$(STAGING_DIR) -C $(SDL_IMAGE_DIR) install;
	touch -c $(STAGING_DIR)/usr/lib/libSDL_image.so
	cp -dpf $(STAGING_DIR)/usr/lib/libSDL_image*.so* $(TARGET_DIR)/usr/lib/
	-$(STRIP) --strip-unneeded $(TARGET_DIR)/usr/lib/libSDL_image.so
	touch $(SDL_IMAGE_DIR)/.installed