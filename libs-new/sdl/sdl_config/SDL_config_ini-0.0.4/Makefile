CC = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-gcc
CXX = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-g++
STRIP = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-strip
AR = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ar
RANLIB = /opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-ranlib
MKDIR = mkdir -p

CFLAGS = `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --cflags` -fPIC
LDFLAGS = `/opt/open2x/gcc-4.1.1-glibc-2.3.6/bin/sdl-config --libs`

OBJS = SDL_config.o SDL_inifile.o
TARGET = libSDL_inifile.so.1.0.0

PREFIX = /opt/open2x/gcc-4.1.1-glibc-2.3.6

all: $(TARGET)

SDL_config.o: ./SDL_config.c
	$(CXX) $(CFLAGS) -c ./SDL_config.c
SDL_inifile.o: ./SDL_inifile.c
	$(CC) $(CFLAGS) -c ./SDL_inifile.c

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -W1,-soname,libSDL_inifile.so.1 -o $(TARGET) $(OBJS)
	$(AR) rc libSDL_inifile.a $(OBJS)
	$(RANLIB) libSDL_inifile.a

install:
	$(MKDIR) $(PREFIX)/lib
	$(MKDIR) $(PREFIX)/include/SDL

	cp -f libSDL_inifile.so.1.0.0 $(PREFIX)/lib/libSDL_inifile.so.1.0.0
	cp -f libSDL_inifile.a $(PREFIX)/lib/libSDL_inifile.a
	cd $(PREFIX)/lib; ln -f -s libSDL_inifile.so.1.0.0 libSDL_inifile.so; ln -f -s libSDL_inifile.so.1.0.0 libSDL_inifile.so.1
	cp -f SDL_inifile.h $(PREFIX)/include/SDL/SDL_inifile.h 

clean:
	rm -f $(OBJS) $(TARGET) libSDL_inifile.a
