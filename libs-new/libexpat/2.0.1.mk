LIBEXPAT_DIR := libexpat/expat-2.0.1

o2x-libexpat:
	@-echo "Build libexpat"
	(cd $(LIBEXPAT_DIR); rm -rf config.cache; \
		./autogen.sh; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBEXPAT_DIR)
	$(FLAGS) make -C $(LIBEXPAT_DIR) install
	$(FLAGS) make -C $(LIBEXPAT_DIR) clean
