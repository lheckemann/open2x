
O_TARGET := idedriver-arm.o

obj-y		:=
obj-m		:=

obj-$(CONFIG_BLK_DEV_IDE_ICSIDE)	+= icside.o
obj-$(CONFIG_BLK_DEV_IDE_RAPIDE)	+= rapide.o
obj-$(CONFIG_BLK_DEV_IDE_RISCSTATION)	+= rstation-ide.o
obj-$(CONFIG_BLK_DEV_IDE_MP2520F)		+= mp2520f.o

EXTRA_CFLAGS	:= -I../

include $(TOPDIR)/Rules.make
