LIBMAD_DIR := libmad/libmad-0.15.1b

o2x-libmad:
	@-echo "Build libmad"
	(cd $(LIBMAD_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--enable-speed\
	)
	$(FLAGS) make -C $(LIBMAD_DIR)
	$(FLAGS) make -C $(LIBMAD_DIR) install
	#$(FLAGS) make -C $(LIBMAD_DIR) clean
