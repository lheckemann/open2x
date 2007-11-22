FREETYPE_DIR := freetype/freetype-2.3.5

o2x-freetype:
	@-echo "Build freetype"
	(cd $(FREETYPE_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(FREETYPE_DIR)
	$(FLAGS) make -C $(FREETYPE_DIR) install
	#$(FLAGS) make -C $(FREETYPE_DIR) clean
