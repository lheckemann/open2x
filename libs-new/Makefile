# Libraries to build
ifeq ($(strip $(BUILD_BUILDROOT)),y)
include .config
else
TARGET_BZIP2 = y
TARGET_FREETYPE = y
TARGET_IMLIB2 = y
TARGET_LIBEXPAT = y
TARGET_LIBICONV = y
TARGET_LIBID3TAG = y
TARGET_LIBJPEG = y
TARGET_LIBMAD = y
TARGET_LIBMIKMOD = y
TARGET_LIBOGG = y
TARGET_LIBPNG = y
TARGET_LIBTIFF = y
TARGET_LIBUNGIF = y
TARGET_LIBVORBIS = y
TARGET_LIBXML2 = y
TARGET_MPEG2DEC = y
TARGET_NCURSES = y
TARGET_TREMOR = y
TARGET_READLINE = y
TARGET_SDL = y
TARGET_SDL_GFX = y
TARGET_SDL_IMAGE = y
TARGET_SDL_MIXER = y
TARGET_SDL_SVG = y
TARGET_SDL_TTF = y
TARGET_SMPEG = y
TARGET_ZLIB = y
endif

# Do not edit below here
FLAGS += CC='$(OPEN2X)/bin/$(HOST)-gcc'
FLAGS += CXX='$(OPEN2X)/bin/$(HOST)-g++'
FLAGS += AR='$(OPEN2X)/bin/$(HOST)-ar'
FLAGS += STRIP='$(OPEN2X)/bin/$(HOST)-strip'
FLAGS += RANLIB='$(OPEN2X)/bin/$(HOST)-ranlib'
FLAGS += OPEN2X='$(PREFIX)'

FLAGS += CFLAGS='-O3 -ffast-math -fomit-frame-pointer -mcpu=arm920t -DARM -D_ARM_ASSEM_ -I$(OPEN2X)/include -I$(OPEN2X)/include/libxml2 -I$(OPEN2X)/include/SDL'
FLAGS += LDFLAGS='-L$(OPEN2X)/lib'
FLAGS += PKG_CONFIG='$(OPEN2X)/bin/pkg-config'

# Build everything
all: world
include ./*/Makefile.mk

world: $(TARGETS)
