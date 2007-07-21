CC = arm-linux-gcc
CXX = arm-linux-g++
STRIP = arm-linux-strip
MKDIR = mkdir -p

CFLAGS = `../arm-linux-sdl-config --cflags` -fPIC
LDFLAGS = `../arm-linux-sdl-config --libs`

OBJS = SDL_config.o SDL_inifile.o
TARGET = libSDL_inifile.so.1.0.0

PREFIX = ../image

all: $(TARGET)

SDL_config.o: ./src/SDL_config.c
	$(CXX) $(CFLAGS) -c ./src/SDL_config.c
SDL_inifile.o: ./SDL_inifile.c
	$(CC) $(CFLAGS) -c ./SDL_inifile.c

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -shared -W1,-soname,libSDL_inifile.so.1 -o $(TARGET) $(OBJS)

install:
	$(MKDIR) $(PREFIX)/lib
	$(MKDIR) $(PREFIX)/include/SDL

	cp -f libSDL_inifile.so.1.0.0 $(PREFIX)/lib/libSDL_inifile.so.1.0.0
	cd $(PREFIX)/lib; ln -f -s libSDL_inifile.so.1.0.0 libSDL_inifile.so; ln -f -s libSDL_inifile.so.1.0.0 libSDL_inifile.so.1
	cp -f SDL_inifile.h $(PREFIX)/include/SDL/SDL_inifile.h 

clean:
	rm -f $(OBJS) $(TARGET)
