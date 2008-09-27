SDL_TTF_DIR := sdl/sdl_ttf/SDL_ttf-2.0.7

o2x-sdl_ttf: o2x-sdl o2x-freetype
	@-echo "Build SDL_ttf"
	(cd $(SDL_TTF_DIR); rm -rf config.cache; \
		$(FLAGS) $(LIBS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-sdl-prefix=$(PREFIX)\
				--with-freetype-prefix=$(PREFIX)\
				--without-x\
	)
	$(FLAGS) make -C $(SDL_TTF_DIR)
	$(FLAGS) make -C $(SDL_TTF_DIR) install
	$(FLAGS) make -C $(SDL_TTF_DIR) clean
