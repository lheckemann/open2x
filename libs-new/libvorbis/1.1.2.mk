LIBVORBIS_DIR := libvorbis/libvorbis-1.1.2

o2x-libvorbis:
	@-echo "Build libvorbis"
	(cd $(LIBVORBIS_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-ogg=$(PREFIX)\
	)
	$(FLAGS) make -C $(LIBVORBIS_DIR)
	$(FLAGS) make -C $(LIBVORBIS_DIR) install
	#$(FLAGS) make -C $(LIBVORBIS_DIR) clean
