LIBOGG_DIR := libogg/libogg-1.1.3

o2x-libogg:
	@-echo "Build libogg"
	(cd $(LIBOGG_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBOGG_DIR)
	$(FLAGS) make -C $(LIBOGG_DIR) install
	#$(FLAGS) make -C $(LIBOGG_DIR) clean
