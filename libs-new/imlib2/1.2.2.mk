IMLIB2_DIR := imlib2/imlib2-1.2.2

o2x-imlib2:
	@-echo "Build imlib2"
	(cd $(IMLIB2_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--with-freetype-config=$(PREFIX)/bin/freetype-config\
				--without-x\
	)
	$(FLAGS) make -C $(IMLIB2_DIR)
	$(FLAGS) make -C $(IMLIB2_DIR) install
	#$(FLAGS) make -C $(IMLIB2_DIR) clean
