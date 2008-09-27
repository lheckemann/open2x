SDL_SVG_DIR := sdl/sdl_svg/SDL_svg-1.1.9

o2x-sdl_svg: o2x-sdl o2x-libxml2
	@-echo "Build SDL_svg"
	(cd $(SDL_SVG_DIR); rm -rf config.cache; \
		$(FLAGS) $(LIBS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-sdl-prefix=$(PREFIX)\
	)
	$(FLAGS) make -C $(SDL_SVG_DIR)
	$(FLAGS) make -C $(SDL_SVG_DIR) install
	$(FLAGS) make -C $(SDL_SVG_DIR) clean
