
 BUILD  = $(shell uname -m)
 HOST   = gp2x
 TARGET = gp2x
 PREFIX = /usr/local/gp2xdev
 FLAGS  = 

 TARGETS += $(PREFIX)/$(TARGET)/include/linux/version.h
 TARGETS += $(PREFIX)/$(TARGET)/include/stdio.h
 TARGETS += $(PREFIX)/bin/$(TARGET)-ar
 TARGETS += $(PREFIX)/bin/$(TARGET)-gcc
 TARGETS += $(PREFIX)/$(TARGET)/lib/libc.a
 TARGETS += $(PREFIX)/bin/$(TARGET)-g++

 all: $(TARGETS)

 clean:
	rm -f $(TARGETS)

 $(PREFIX)/$(TARGET)/include/linux/version.h:
	mkdir -p $(PREFIX)/$(TARGET)/include/asm
	cp -RHp linux-headers-2.6.12.0/include/asm-arm/*.h $(PREFIX)/$(TARGET)/include/asm
	mkdir -p $(PREFIX)/$(TARGET)/include/linux
	cp -RHp linux-headers-2.6.12.0/include/linux/*.h $(PREFIX)/$(TARGET)/include/linux

 $(PREFIX)/$(TARGET)/include/stdio.h: $(PREFIX)/$(TARGET)/include/linux/version.h
	mkdir -p glibc-2.3.5/build-$(TARGET)-stage1
	cd glibc-2.3.5/build-$(TARGET)-stage1 && ../configure --prefix=$(PREFIX)/$(TARGET) --build=$(BUILD) --host=$(HOST) --disable-nls --enable-add-ons=linuxthreads --enable-omitfp --enable-shared --with-headers=$(PREFIX)/$(TARGET)/include --without-fp && make install-headers
	touch $(PREFIX)/$(TARGET)/include/gnu/stubs.h
	cp glibc-2.3.5/build-$(TARGET)-stage1/bits/stdio_lim.h $(PREFIX)/$(TARGET)/include/bits/stdio_lim.h

 $(PREFIX)/bin/$(TARGET)-ar: $(PREFIX)/$(TARGET)/include/stdio.h
	@echo  $(PREFIX)/bin/$(TARGET)-ar
	mkdir -p binutils-2.16.1/build-$(TARGET)
	cd binutils-2.16.1/build-$(TARGET) && ../configure --prefix=$(PREFIX) --target=$(TARGET) --disable-nls --disable-shared && make && make install && make clean

 $(PREFIX)/bin/$(TARGET)-gcc: $(PREFIX)/bin/$(TARGET)-ar
	mkdir -p gcc-4.0.2/build-$(TARGET)-stage1
	cd gcc-4.0.2/build-$(TARGET)-stage1 && ../configure --prefix=$(PREFIX) --target=$(TARGET) --disable-libmudflap --disable-libssp --disable-libstdcxx-pch --disable-multilib --disable-nls --disable-shared --enable-c99 --enable-__cxa_atexit --enable-cxx-flags=-mcpu=arm920t --enable-languages='c' --enable-long-long --enable-symvers=gnu --enable-threads=posix --with-cpu=arm920t --with-float=soft && make all-gcc && make install-gcc && make clean

 $(PREFIX)/$(TARGET)/lib/libc.a: $(PREFIX)/bin/$(TARGET)-gcc $(PREFIX)/bin/$(TARGET)-ar
	mkdir -p glibc-2.3.5/build-$(TARGET)-stage2
	cd glibc-2.3.5/build-$(TARGET)-stage2 && ../configure --prefix=$(PREFIX)/$(TARGET) --build=$(BUILD) --host=$(HOST) --disable-nls --enable-add-ons=linuxthreads --enable-omitfp --enable-shared --with-headers=$(PREFIX)/$(TARGET)/include --without-fp && make && make install && make clean

 $(PREFIX)/bin/$(TARGET)-g++: $(PREFIX)/$(TARGET)/lib/libc.a
	mkdir -p gcc-4.0.2/build-$(TARGET)-stage2
	cd gcc-4.0.2/build-$(TARGET)-stage2 && ../configure --prefix=$(PREFIX) --target=$(TARGET) --disable-libmudflap --disable-libssp --disable-libstdcxx-pch --disable-multilib --disable-nls --disable-shared --enable-c99 --enable-__cxa_atexit --enable-cxx-flags=-mcpu=arm920t --enable-languages='c,c++' --enable-long-long --enable-symvers=gnu --enable-threads=posix --with-cpu=arm920t --with-float=soft && make all && make install && make clean
