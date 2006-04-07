
  CC      = open2x-gcc
  CFLAGS  = -static -Wall -pedantic -std=c99 -Os $(shell $(OPEN2X)/bin/sdl-config --cflags)
  LDFLAGS = $(shell $(OPEN2X)/bin/sdl-config --libs) -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lvorbisidec -lsmpeg -lpng -lz -ljpeg -lfreetype -lm
  STRIP   = open2x-strip --strip-all
  PREFIX  = /media/GP2X/Demos/hello

  all: hello.gpe

  default: all

  clean:
	rm -f hello.gpe

  hello.gpe: hello.c
	$(CC) $(CFLAGS) hello.c -o hello.gpe $(LDFLAGS) $(LDFLAGS)
	$(STRIP) hello.gpe

  install: hello.gpe
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/media
	cp hello.gpe $(PREFIX)
	cp media/* $(PREFIX)/media
	sync
