LIBUNGIF_DIR := libungif/libungif-4.1.4

o2x-libungif:
	@-echo "Build libungif"
	(cd $(LIBUNGIF_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBUNGIF_DIR)
	$(FLAGS) make -C $(LIBUNGIF_DIR) install
	#$(FLAGS) make -C $(LIBUNGIF_DIR) clean
