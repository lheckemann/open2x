LIBMPCDEC_DIR := libmpcdec/libmpcdec-1.2.6

o2x-libmpcdec:
	@-echo "Build libmpcdec"
	(cd $(LIBMPCDEC_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBMPCDEC_DIR)
	$(FLAGS) make -C $(LIBMPCDEC_DIR) install
	$(FLAGS) make -C $(LIBMPCDEC_DIR) clean
