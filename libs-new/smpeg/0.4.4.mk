SMPEG_DIR := smpeg/smpeg-0.4.4

o2x-smpeg: o2x-sdl
	@-echo "Build smpeg"
	(cd $(SMPEG_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--disable-gtk-player\
				--disable-opengl-player\
				--with-sdl-prefix=$(PREFIX)\
				--without-x\
	)
	$(FLAGS) make -C $(SMPEG_DIR)
	$(FLAGS) make -C $(SMPEG_DIR) install
	$(FLAGS) make -C $(SMPEG_DIR) clean
