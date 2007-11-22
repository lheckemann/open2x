NCURSES_DIR := ncurses/ncurses-5.5

o2x-ncurses:
	@-echo "Build ncurses"
	(cd $(NCURSES_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(NCURSES_DIR)
	$(FLAGS) make -C $(NCURSES_DIR) install
	#$(FLAGS) make -C $(NCURSES_DIR) clean
