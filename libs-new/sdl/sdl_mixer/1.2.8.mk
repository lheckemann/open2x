SDL_MIXER_DIR := sdl/sdl_mixer/SDL_mixer-1.2.8

o2x-sdl_mixer: o2x-sdl o2x-libogg o2x-libmikmod o2x-libogg o2x-libid3tag o2x-tremor o2x-smpeg 
	@-echo "Build SDL_mixer"
	(cd $(SDL_MIXER_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-sdl-prefix=$(PREFIX)\
				--enable-music-mp3=yes\
				--enable-music-mod=yes\
				--enable-music-libmikmod=yes\
				--enable-music-ogg-tremor=yes\
	)
	$(FLAGS) make -C $(SDL_MIXER_DIR)
	$(FLAGS) make -C $(SDL_MIXER_DIR) install
	$(FLAGS) make -C $(SDL_MIXER_DIR) clean
