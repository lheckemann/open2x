BZIP2_DIR := bzip2/bzip2-1.0.5

o2x-bzip2:
	@-echo "Build bzip2"
	$(FLAGS) make -C $(BZIP2_DIR)
	$(FLAGS) make -C $(BZIP2_DIR) install
	$(FLAGS) make -C $(BZIP2_DIR) clean
