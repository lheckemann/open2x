############################################
#
# SDL
#      Download and compile the latest HW 
#      SDL code from the SVN.
#
############################################

SDL_SITE:=https://svn.sourceforge.net/svnroot/open2x/trunk/libs/SDL-1.2.9/

$(SDL_DIR)/.unpacked:
	# Download and prepare the source
	$(SVN) $(SDL_SITE) $(SDL_DIR)
	(cd $(SDL_DIR); \
	sh autogen.sh );
	touch $(SDL_DIR)/.unpacked

$(SDL_DIR)/.configured: $(SDL_DIR)/.unpacked
	(cd $(SDL_DIR); \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)" \
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
		--enable-static \
		--enable-pthreads \
		--enable-pthreads-sem \
		--enable-threads \
		--disable-arts \
		--disable-esd \
		--disable-nasm \
		--disable-video-x11 );
	touch $(SDL_DIR)/.configured

$(SDL_DIR)/.compiled: $(SDL_DIR)/.configured
	$(MAKE) -C $(SDL_DIR)
	touch $(SDL_DIR)/.compiled

$(SDL_DIR)/.installed: $(SDL_DIR)/.compiled
	$(MAKE) DESTDIR=$(STAGING_DIR) -C $(SDL_DIR) install;
	touch -c $(STAGING_DIR)/usr/lib/libSDL.so
	cp -dpf $(STAGING_DIR)/usr/lib/libSDL*.so* $(TARGET_DIR)/usr/lib/
	-$(STRIP) --strip-unneeded $(TARGET_DIR)/usr/lib/libSDL.so
	touch $(SDL_DIR)/.installed