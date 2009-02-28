CFLAGS=-c -O2 -g -Wall -DPLATFORM_GP2X -mcpu=arm920t -mtune=arm920 -ffast-math -W -Wall -pthread -msoft-float

#LDFLAGS=-L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -static -lgcc -lm -lc -ldl -larmmem -msoft-float -O2
LDFLAGS=-L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -Wl,-rpath,/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib -lgcc -lm -lc -ldl -larmmem -msoft-float -O2

SOURCES=buttonquery.c


OBJECTS=$(SOURCES:.c=.o)
EXECUTABLEGP=buttonquery

CC=arm-open2x-linux-gcc
STRIP=arm-open2x-linux-strip
CFLAGS+=-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include/ \
		-I/opt/open2x/gcc-4.1.1-glibc-2.3.6/arm-open2x-linux/include/ \
		-D_REENTRANT -msoft-float -DPLATFORM_GP2X

all: 
	make gpversion

gpversion: $(SOURCES) $(EXECUTABLEGP)


$(EXECUTABLEGP): $(OBJECTS) 
#	$(CC) $(LDFLAGS) -static $(OBJECTS) -o $@ $(GPLDFLAGS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(GPLDFLAGS)
	$(STRIP) $@



.c.o:
	$(CC) $(CFLAGS) $< -o $@

cleanobjs:
	rm -f $(OBJECTS)

clean:
	-rm -f buttonquery *~ *.o *.bak 

