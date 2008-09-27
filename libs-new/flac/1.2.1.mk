FLAC_DIR := flac/flac-1.2.1

o2x-flac:
	@-echo "Build flac"
	(cd $(FLAC_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(FLAC_DIR)
	$(FLAGS) make -C $(FLAC_DIR) install
	$(FLAGS) make -C $(FLAC_DIR) clean
