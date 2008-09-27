LIBXML2_DIR := libxml2/libxml2-2.6.30

o2x-libxml2:
	@-echo "Build libxml2"
	(cd $(LIBXML2_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBXML2_DIR)
	$(FLAGS) make -C $(LIBXML2_DIR) install
	$(FLAGS) make -C $(LIBXML2_DIR) clean
