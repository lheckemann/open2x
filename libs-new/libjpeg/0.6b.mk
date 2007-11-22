LIBJPEG_DIR := libjpeg/libjpeg-0.6b

o2x-libjpeg:
	@-echo "Build libjpeg"
	(cd $(LIBJPEG_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBJPEG_DIR)
	$(FLAGS) make -C $(LIBJPEG_DIR) install-lib
	$(FLAGS) make -C $(LIBJPEG_DIR) install-headers
	#$(FLAGS) make -C $(LIBJPEG_DIR) clean
