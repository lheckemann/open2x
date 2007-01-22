CFLAGS	= -g -Wall -pedantic -Wmissing-prototypes
#CFLAGS	= -O2 -Wall -pedantic -Wmissing-prototypes
OBJS = main.o jtag.o debug.o memory.o parse.o regs.o

open2x-jtag: $(OBJS)
	$(CC) -g -o open2x-jtag $(OBJS)

clean:
	rm -f *.o *~ core open2x-jtag open2x-jtag.exe

debug:
	gdb open2x-jtag
