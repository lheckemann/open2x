LIBPNG_DIR := libpng/libpng-1.2.23

o2x-libpng: o2x-zlib
	@-echo "Build libpng"
	(cd $(LIBPNG_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBPNG_DIR)
	$(FLAGS) make -C $(LIBPNG_DIR) install
	#$(FLAGS) make -C $(LIBPNG_DIR) clean
