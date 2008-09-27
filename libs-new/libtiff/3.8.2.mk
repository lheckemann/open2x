LIBTIFF_DIR := libtiff/libtiff-3.8.2

o2x-libtiff:
	@-echo "Build libtiff"
	(cd $(LIBTIFF_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBTIFF_DIR)
	$(FLAGS) make -C $(LIBTIFF_DIR) install
	$(FLAGS) make -C $(LIBTIFF_DIR) clean
