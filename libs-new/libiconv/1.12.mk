LIBICONV_DIR := libiconv/libiconv-1.12

o2x-libiconv:
	@-echo "Build libiconv"
	(cd $(LIBICONV_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBICONV_DIR)
	$(FLAGS) make -C $(LIBICONV_DIR) install
	#$(FLAGS) make -C $(LIBICONV_DIR) clean
