# BUILD SETTINGS ###################################
DEBUG := 0
# Valid values: UNIX, GP2X
PLATFORM := GP2X

# If building for the GP2X
GP2X_CHAINPREFIX := /opt/open2x/gcc-4.1.1-glibc-2.3.6
GP2X_CHAIN := $(GP2X_CHAINPREFIX)/bin/arm-open2x-linux-

# END SETTINGS #####################################

TARGET := o2xiv
OBJS := files.o font.o image.o input.o main.o menu.o prefs.o scale.o thread.o

ifeq ($(DEBUG), 1)
	DEBUG_FLAGS := -g3 -O0
else
	DEBUG_FLAGS := -O3 -DNDEBUG
endif

ifeq ($(PLATFORM), GP2X)
	SDL_CFLAGS := `$(GP2X_CHAINPREFIX)/bin/sdl-config --cflags` -I$(GP2X_CHAINPREFIX)/include -DTARGET_GP2X -mcpu=arm920t -mtune=arm920t -ffast-math
	SDL_LDFLAGS := `$(GP2X_CHAINPREFIX)/bin/sdl-config --libs` -lSDL_image -ljpeg -L$(GP2X_CHAINPREFIX)/lib
	CC := $(GP2X_CHAIN)gcc
	STRIP := $(GP2X_CHAIN)strip
	TARGET := $(TARGET).gpu
endif
ifeq ($(PLATFORM), UNIX)
	SDL_CFLAGS := $(shell sdl-config --cflags) -DTARGET_UNIX
	SDL_LDFLAGS := $(shell sdl-config --libs) -lSDL_image -ljpeg
endif

CFLAGS := --std=c99 -pedantic -Wall -Wextra -Werror -I$(CURDIR)/src/ $(DEBUG_FLAGS) $(SDL_CFLAGS)
LDFLAGS := $(SDL_LDFLAGS)

####################################################

all : $(TARGET)

OBJS := $(foreach obj, $(OBJS), obj/$(obj))

$(TARGET) : $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

ifneq ($(MAKECMDGOALS), clean)
-include $(OBJS:.o=.d)
endif

obj/%.d : obj/%.o
obj/%.o : src/%.c
	$(CC) -o $@ -MMD -c $(CFLAGS) $<

.PHONY : clean

clean :
	rm -f obj/*.o obj/*.d
	rm -f $(TARGET)
