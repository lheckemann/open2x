LIBID3TAG_DIR := libid3tag/libid3tag-0.15.1b

o2x-libid3tag: o2x-zlib
	@-echo "Build libid3tag"
	(cd $(LIBID3TAG_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(LIBID3TAG_DIR)
	$(FLAGS) make -C $(LIBID3TAG_DIR) install
	$(FLAGS) make -C $(LIBID3TAG_DIR) clean
