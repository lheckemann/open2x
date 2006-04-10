
 BUILD   = $(shell uname -m)
 HOST    = open2x
 TARGET  = open2x
 PREFIX  = $(OPEN2X)

 CFLAGS  = -I$(PREFIX)/include -I$(PREFIX)/include/libxml2
 LDFLAGS = -L$(PREFIX)/lib -R$(PREFIX)/lib
 CC      = $(HOST)-gcc $(CFLAGS)
 CXX     = $(HOST)-g++ $(CFLAGS)
 FLAGS   = CC='$(CC)' CXX='$(CXX)' CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)'

 TARGETS = busybox lua python

 all: $(TARGETS)

 clean:
	rm -f $(TARGETS)

 busybox:
	cd busybox-1.1.1 && make CROSS=$(TARGET)-
	mkdir -p $(PREFIX)/firmware/usr/bin
	cp busybox-1.1.1/busybox $(PREFIX)/firmware/usr/bin

 lua:
	cd lua-5.1 && make && make

 python:
	mkdir -p python-2.4.3/build && cd python-2.4.3/build && $(FLAGS) ../configure --prefix=$(PREFIX) --build=$(BUILD) --host=$(HOST) --with-shared --with-static && make
