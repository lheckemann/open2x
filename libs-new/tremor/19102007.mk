TREMOR_DIR := tremor/tremor-19102007

o2x-tremor:
	@-echo "Build tremor"
	(cd $(TREMOR_DIR); rm -rf config.cache; \
		./autogen.sh; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(TREMOR_DIR)
	$(FLAGS) make -C $(TREMOR_DIR) install
	$(FLAGS) make -C $(TREMOR_DIR) clean
