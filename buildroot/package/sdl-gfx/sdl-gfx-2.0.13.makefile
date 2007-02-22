############################################
#
# SDL_gfx
#
############################################

$(SDL_GFX_DIR)/.unpacked:
	# Download and prepare the source
	$(WGET) -P $(DL_DIR) http://www.ferzkopp.net/Software/SDL_gfx-2.0/SDL_gfx-2.0.13.tar.gz
	$(ZCAT) $(DL_DIR)/SDL_gfx-2.0.13.tar.gz | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	touch $(SDL_GFX_DIR)/.unpacked

$(SDL_GFX_DIR)/.configured: $(SDL_GFX_DIR)/.unpacked
	(cd $(SDL_GFX_DIR); \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)" \
	./configure \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=`uname -m` \
		--with-sdl-prefix=$(STAGING_DIR) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--includedir=/include \
		--mandir=/usr/man \
		--libdir=/lib \
		--infodir=/usr/info \
		--enable-shared \
		--enable-static \
		--disable-mmx );
	sed -e 's/-L\/lib -Wl,-rpath,\/usr\/lib -lSDL -lpthread/-L\/opt\/open2x\/gcc-3.4.4-glibc-2.3.6\/lib -Wl,-rpath,\/usr\/lib -lSDL -lpthread/g' $(SDL_GFX_DIR)/Makefile > $(SDL_GFX_DIR)/Makefile.temp
	mv $(SDL_GFX_DIR)/Makefile.temp $(SDL_GFX_DIR)/Makefile
#	sed -e 's/-L\/lib -Wl,-rpath,\/usr\/lib -lSDL -lpthread/-L$(STAGING_DIR)\/lib -Wl,-rpath,\/usr\/lib -lSDL -lpthread/g' Makefile
	touch $(SDL_GFX_DIR)/.configured

$(SDL_GFX_DIR)/.compiled: $(SDL_GFX_DIR)/.configured
	CFLAGS="$CFLAGS -L$(STAGING_DIR)/lib" $(MAKE) -C $(SDL_GFX_DIR)
	touch $(SDL_GFX_DIR)/.compiled

$(SDL_GFX_DIR)/.installed: $(SDL_GFX_DIR)/.compiled
	$(MAKE) DESTDIR=$(STAGING_DIR) -C $(SDL_GFX_DIR) install;
	touch -c $(STAGING_DIR)/usr/lib/libSDL_gfx.so
	cp -dpf $(STAGING_DIR)/usr/lib/libSDL_gfx*.so* $(TARGET_DIR)/usr/lib/
	-$(STRIP) --strip-unneeded $(TARGET_DIR)/usr/lib/libSDL_gfx.so
	touch $(SDL_GFX_DIR)/.installed