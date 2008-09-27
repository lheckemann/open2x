MPEG2DEC_DIR_ROOT := mpeg2dec
MPEG2DEC_DIR := mpeg2dec/libmpeg2-0.5.1

o2x-mpeg2dec: #o2x-sdl
	@-echo "Build mpeg2dec"

	@-(cd $(MPEG2DEC_DIR_ROOT); \
	echo " extracting mpeg2dec"; \
	tar -zxf libmpeg2-0.5.1.tar.gz; \
	echo " commenting pld instructions"; \
	sed s/pld/@pld/ libmpeg2-0.5.1/libmpeg2/motion_comp_arm_s.S > libmpeg2-0.5.1/libmpeg2/motion_comp_arm_s.S.new && mv libmpeg2-0.5.1/libmpeg2/motion_comp_arm_s.S.new libmpeg2-0.5.1/libmpeg2/motion_comp_arm_s.S )

	(cd $(MPEG2DEC_DIR); rm -rf config.cache; \
		$(FLAGS) ./configure --prefix=$(PREFIX)\
				--target=$(TARGET)\
				--host=$(HOST)\
				--build=$(BUILD)\
				--enable-shared\
				--enable-static\
				--disable-sdl\
				--with-sdl-prefix=$(PREFIX)\
	)
	$(FLAGS) make -C $(MPEG2DEC_DIR)
	$(FLAGS) make -C $(MPEG2DEC_DIR) install
	$(FLAGS) make -C $(MPEG2DEC_DIR) clean
