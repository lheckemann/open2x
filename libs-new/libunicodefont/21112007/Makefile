CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-gcc
AR = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ar
RANLIB = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ranlib

all: UnicodeFontCode.o UnicodeFontSize.o UnicodeFont.o libunicodefont.so.1.0.1 install

UnicodeFontCode.o:
	$(CC) -fPIC -c UnicodeFontCode.c
UnicodeFontSize.o:
	$(CC) -fPIC -c UnicodeFontSize.c 
UnicodeFont.o:
	$(CC) -fPIC -c UnicodeFont.c
libunicodefont.so.1.0.1:
	$(CC) -shared -W1,-soname,libunicodefont.so.1 -o libunicodefont.so.1.0.1 UnicodeFontCode.o UnicodeFontSize.o UnicodeFont.o
	$(AR) rc libunicodefont.a UnicodeFontCode.o UnicodeFontSize.o UnicodeFont.o
	$(RANLIB) libunicodefont.a

install:
	mkdir -p /opt/open2x/gcc-4.1.1-glibc-2.3.6/lib
	mkdir -p /opt/open2x/gcc-4.1.1-glibc-2.3.6/include
	cp -f libunicodefont.so.1.0.1 /opt/open2x/gcc-4.1.1-glibc-2.3.6/lib/libunicodefont.so.1.0.1
	cp -f libunicodefont.a /opt/open2x/gcc-4.1.1-glibc-2.3.6/lib/libunicodefont.a
	cp -f UnicodeFont.h /opt/open2x/gcc-4.1.1-glibc-2.3.6/include/UnicodeFont.h

	cd /opt/open2x/gcc-4.1.1-glibc-2.3.6/lib && ln -s libunicodefont.so.1.0.1 libunicodefont.so

clean:
	rm -f *.o
	rm -rf lib*
