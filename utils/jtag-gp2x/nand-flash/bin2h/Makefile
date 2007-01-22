
O_TARGET = bin2h
OBJS = bin2h.o

all : $(O_TARGET)
	./bin2h gp2xboot.img ../include/bios.h

clean : 
	rm -f $(OBJS) $(O_TARGET)

$(O_TARGET): $(OBJS) Makefile
	gcc -O2 -s -o bin2h $(OBJS)
	rm -f $(OBJS)

bin2h.o : bin2h.c Makefile
	gcc -O2 -c -o bin2h.o bin2h.c

