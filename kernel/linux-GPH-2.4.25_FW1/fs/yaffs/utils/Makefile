#Makefile for mkyaffs
#
# NB this is not yet suitable for putting into the kernel tree.
# YAFFS: Yet another FFS. A NAND-flash specific file system. 
#
# Copyright (C) 2002 Aleph One Ltd.
#   for Toby Churchill Ltd and Brightstar Engineering
#
# Created by Charles Manning <charles@aleph1.co.uk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

## Change or override  KERNELDIR to your kernel

# target kernel
T_KERNELDIR = ../../../

# host kernel
H_KERELDIR = /usr/src/linux

CFLAGS =   -I.. -O2 -Wall -DCONFIG_YAFFS_UTIL
CFLAGS+=   -Wshadow -Wpointer-arith -Wwrite-strings -Wstrict-prototypes -Wmissing-declarations
CFLAGS+=   -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Winline


MKYAFFSOBJS = mkyaffs.o

MKYAFFSIMAGEOBJS = mkyaffsimage.o yaffs_ecc.o

CROSS=/opt/host/armv4l/bin/armv4l-redhat-linux-


all: mkyaffs mkyaffsimage mmaptest

$(MKYAFFSIMAGEOBJS): %.o: %.c
	gcc -c -I$(H_KERNELDIR)/include $(CFLAGS) $< -o $@

mkyaffsimage: $(MKYAFFSIMAGEOBJS)
	gcc -s -o $@ $(MKYAFFSIMAGEOBJS)

$(MKYAFFSOBJS) mmaptest.o: %.o: %.c
	$(CROSS)gcc -c -I$(T_KERNELDIR)/include $(CFLAGS) $< -o $@

mkyaffs: $(MKYAFFSOBJS)
	$(CROSS)gcc -s -o $@ $(MKYAFFSOBJS)

mmaptest: mmaptest.o
	$(CROSS)gcc -s -o $@ mmaptest.o

yaffs_ecc.c:
	ln -s ../yaffs_ecc.c yaffs_ecc.c


clean:
	rm -vf *.o core mkyaffs mkyaffsimage mmaptest
