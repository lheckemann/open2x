FAAD_DIR := faad/faad2-2.6.1

o2x-faad:
	@-echo "Build faad"
	(cd $(FAAD_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=`uname -m`\
	)
	$(FLAGS) make -C $(FAAD_DIR)
	$(FLAGS) make -C $(FAAD_DIR) install
	$(FLAGS) make -C $(FAAD_DIR) clean
