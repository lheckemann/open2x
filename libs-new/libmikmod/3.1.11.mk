LIBMIKMOD_DIR := libmikmod/libmikmod-3.1.11

o2x-libmikmod:
	@-echo "Build libmikmod"
	(cd $(LIBMIKMOD_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--disable-esd\
	)
	$(FLAGS) make -C $(LIBMIKMOD_DIR)
	$(FLAGS) make -C $(LIBMIKMOD_DIR) install
	#$(FLAGS) make -C $(LIBMIKMOD_DIR) clean
