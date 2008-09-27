SDL_DIR := sdl/sdl/SDL-1.2.11

o2x-sdl:
	@-echo "Build SDL"
	(cd $(SDL_DIR); rm -rf config.cache; \
		./autogen.sh; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--oldincludedir=$(PREFIX)/arm-open2x-linux/include\
				--enable-shared\
				--enable-static\
				--enable-pthreads\
				--enable-pthreads-sem\
				--enable-threads\
				--disable-video-directfb\
				--disable-video-x11\
				--disable-arts\
				--disable-esd\
				--enable-video\
				--enable-video-gp2x\
	)
	$(FLAGS) make -C $(SDL_DIR)
	$(FLAGS) make -C $(SDL_DIR) install
	$(FLAGS) make -C $(SDL_DIR) clean
