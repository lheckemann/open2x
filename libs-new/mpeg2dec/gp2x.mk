MPEG2DEC_DIR := mpeg2dec/mpeg2dec-gp2x

o2x-mpeg2dec: o2x-sdl
	@-echo "Build mpeg2dec"
	(cd $(MPEG2DEC_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-sdl-prefix=$(PREFIX)\
	)
	$(FLAGS) make -C $(MPEG2DEC_DIR)
	$(FLAGS) make -C $(MPEG2DEC_DIR) install
	#$(FLAGS) make -C $(MPEG2DEC_DIR) clean
