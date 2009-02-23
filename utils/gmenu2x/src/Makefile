OPEN2X=/opt/open2x/gcc-4.1.1-glibc-2.3.6
CHAINPREFIX=$(OPEN2X)
CROSS_COMPILE=$(CHAINPREFIX)/bin/arm-open2x-linux-

CC= $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip

CFLAGS = -I./google -I"$(CHAINPREFIX)/include" `$(CHAINPREFIX)/bin/sdl-config --cflags` -DTARGET_GP2X -DOPEN2X -DTIXML_USE_STL -O3 -msoft-float -ffast-math -funroll-loops -Wall -Wno-unknown-pragmas -Wno-format
#CFLAGS = -I./google -I"$(CHAINPREFIX)/include" `$(CHAINPREFIX)/bin/sdl-config --cflags` -g -DTARGET_GP2X -DOPEN2X -DTIXML_USE_STL -O0 -msoft-float -ffast-math -funroll-loops -Wall -Wno-unknown-pragmas -Wno-format
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L$(CHAINPREFIX)/lib `$(CHAINPREFIX)/bin/sdl-config --libs` -lfreetype -lSDL_image -lSDL_gfx -ljpeg -lpng12 -lSDL -lz -ldl # -lpnd

TARGET = ../build/gmenu2x
OBJDIR = ../objs/gp2x

SOURCES := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp, $(OBJDIR)/%.o, $(SOURCES))
#TinyXml
TXML_SOURCES := $(wildcard tinyxml/*.cpp)
TXML_OBJS := $(patsubst %.cpp, $(OBJDIR)/%.o, $(TXML_SOURCES))

#File types rules
$(OBJDIR)/tinyxml/%.o: tinyxml/%.cpp
	$(CXX) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/tinyxml/%.o: tinyxml/%.c
	$(CXX) $(CFLAGS) -o $@ -c $<
$(OBJDIR)/%.o: %.cpp %.h
	$(CXX) $(CFLAGS) -o $@ -c $<

all: dir shared

dir:
	@if [ ! -d $(OBJDIR)/tinyxml ]; then mkdir -p $(OBJDIR)/tinyxml; fi

static: $(TARGET)

$(TARGET): $(OBJS) $(TXML_OBJS) 
	@echo "Linking $(TARGET)..."
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJS) $(TXML_OBJS)
	$(STRIP) $(TARGET)

shared: $(OBJS) $(TXML_OBJS) 
	@echo "Linking $(TARGET)..."
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJS) $(TXML_OBJS)
	$(STRIP) $(TARGET)

clean:
	-rm -f $(OBJDIR)/*.o $(OBJDIR)/tinyxml/*.o $(TARGET)

depend:
	makedepend -p$(OBJDIR)/ -- $(CFLAGS) -- *.cpp
# DO NOT DELETE

