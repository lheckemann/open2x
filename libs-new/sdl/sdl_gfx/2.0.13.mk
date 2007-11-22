SDL_GFX_DIR := sdl/sdl_gfx/SDL_gfx-2.0.13

o2x-sdl_gfx: o2x-sdl
	@-echo "Build SDL_gfx"
	(cd $(SDL_GFX_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-sdl-prefix=$(PREFIX)\
				--disable-mmx\
	)
	$(FLAGS) make -C $(SDL_GFX_DIR)
	$(FLAGS) make -C $(SDL_GFX_DIR) install
	#$(FLAGS) make -C $(SDL_GFX_DIR) clean
