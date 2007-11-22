ZLIB_DIR := zlib/zlib-1.2.3

o2x-zlib:
	@-echo "Build static zlib"
	(cd $(ZLIB_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				#--target=$(TARGET)\
				#--host=$(HOST)\
				#--build=$(BUILD)\
	)
	$(FLAGS) make -C $(ZLIB_DIR)
	$(FLAGS) make -C $(ZLIB_DIR) install
	$(FLAGS) make -C $(ZLIB_DIR) clean

	@-echo "Build shared zlib"
	(cd $(ZLIB_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--shared\
	)
	$(FLAGS) make -C $(ZLIB_DIR)
	$(FLAGS) make -C $(ZLIB_DIR) install
	#$(FLAGS) make -C $(ZLIB_DIR) clean
