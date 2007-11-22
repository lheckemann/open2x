# Libraries to build
ifeq ($(strip $(BUILD_BUILDROOT)),y)
include .config
else
TARGETS+=o2x-zlib
TARGET_BZIP2 = n
TARGET_FREETYPE = n
TARGET_IMLIB2 = n
TARGET_LIBICONV = n
TARGET_LIBID3TAG = n
TARGET_LIBJPEG = n
TARGET_LIBMAD = n
TARGET_LIBMIKMOD = n
TARGET_LIBOGG = n
TARGET_LIBPNG = n
TARGET_LIBTIFF = n
TARGET_LIBUNGIF = n
TARGET_LIBVORBIS = n
TARGET_LIBXML2 = n
TARGET_MPEG2DEC = n
TARGET_NCURSES = n
TARGET_TREMOR = n
TARGET_READLINE = n
TARGET_SDL = n
TARGET_SDL_GFX = n
TARGET_SDL_IMAGE = n
TARGET_SDL_MIXER = n
TARGET_SDL_SVG = n
TARGET_SDL_TTF = n
TARGET_SMPEG = n
TARGET_ZLIB = n
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
