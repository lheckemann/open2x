CFLAGS=-c -O2 -g -Wall -DPLATFORM_GP2X -mcpu=arm920t -mtune=arm920 -ffast-math -W -Wall -pthread -msoft-float

LDFLAGS=-L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -static -lgcc -lm -lc -ldl -larmmem -msoft-float -O2

SOURCES=open2x_volumescale.c


OBJECTS=$(SOURCES:.c=.o)
EXECUTABLEGP=open2x_volumescale.gpe

CC=arm-open2x-linux-gcc
CFLAGS+=-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/ \
		-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/include/ \
		-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/SDL -D_REENTRANT \
		-msoft-float -DPLATFORM_GP2X

all: 
	make gpversion

gpversion: $(SOURCES) $(EXECUTABLEGP)


$(EXECUTABLEGP): $(OBJECTS) 
	$(CC) $(LDFLAGS) -static $(OBJECTS) -o $@ $(GPLDFLAGS)


.c.o:
	$(CC) $(CFLAGS) $< -o $@

cleanobjs:
	rm -f $(OBJECTS)

clean:
	-rm -f open2x_volumescale open2x_volumescale.gpe *~ *.o *.bak 

