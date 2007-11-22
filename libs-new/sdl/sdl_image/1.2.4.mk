SDL_IMAGE_DIR := sdl/sdl_image/SDL_image-1.2.6

o2x-sdl_image: o2x-sdl o2x-imlib2 o2x-libjpeg o2x-libpng o2x-libtiff o2x-libungif
	@-echo "Build SDL_image"
	(cd $(SDL_IMAGE_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-sdl-prefix=$(PREFIX)\
				--disable-mmx\
	)
	$(FLAGS) make -C $(SDL_IMAGE_DIR)
	$(FLAGS) make -C $(SDL_IMAGE_DIR) install
	#$(FLAGS) make -C $(SDL_IMAGE_DIR) clean
