
  CC      = open2x-gcc
  STRIP   = open2x-strip --strip-all
  CFLAGS  = -static -Wall -pedantic -std=c99 -Os $(shell $(OPEN2X)/bin/sdl-config --cflags)
  LDFLAGS = -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lsmpeg $(shell $(OPEN2X)/bin/sdl-config --libs) -lvorbisidec -lpng -lz -ljpeg -lfreetype -lm -lvorbisidec
  PREFIX  = /media/GP2X/Demos/hello

  all: hello.gpe

  default: all

  clean:
	rm -f hello.gpe

  hello.gpe: hello.c
	$(CC) $(CFLAGS) hello.c -o hello.gpe $(LDFLAGS)
	$(STRIP) hello.gpe

  install: hello.gpe
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/media
	cp hello.gpe $(PREFIX)
	cp media/* $(PREFIX)/media
	sync
