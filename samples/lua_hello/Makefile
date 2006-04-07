
  STRIP  = open2x-strip --strip-all
  PREFIX = /media/GP2X/Demos/lua_hello

  all: lua.gpe

  install: all
	mkdir -p $(PREFIX)
	cp lua.gpe $(PREFIX)
	cp wrapper.gpe $(PREFIX)
	cp hello.lua $(PREFIX)

  clean:
	rm -f lua.gpe

  lua.gpe: $(OPEN2X)/bin/lua
	cp $(OPEN2X)/bin/lua lua.gpe
	$(STRIP) lua.gpe
	sync
