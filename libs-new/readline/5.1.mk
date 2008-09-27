READLINE_DIR := readline/readline-5.1

o2x-readline:
	@-echo "Build readline"
	(cd $(READLINE_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
	)
	$(FLAGS) make -C $(READLINE_DIR)
	$(FLAGS) make -C $(READLINE_DIR) install
	$(FLAGS) make -C $(READLINE_DIR) clean
