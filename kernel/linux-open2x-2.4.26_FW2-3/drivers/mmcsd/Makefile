#
# Makefile for the kernel MMC/SD Card subsystem
#

O_TARGET := mmcsd.o

export-objs := mmcsd_bus.o

list-multi := mmcsd_core.o
mmcsd_core-objs := mmcsd_bus.o mmcsd_cs.o

list-multi := mmcsd_slot.o
ifeq ($(CONFIG_ARCH_S3C2440),y)
  mmcsd_slot-objs := mmcsd_slot_s3c2440.o
endif
ifeq ($(CONFIG_ARCH_S3C2410),y)
  mmcsd_slot-objs := mmcsd_slot_s3c2410.o
endif
ifeq ($(CONFIG_ARCH_MMSP2),y)
  mmcsd_slot-objs := mmcsd_slot_mmsp2.o
endif

obj-$(CONFIG_MMCSD) += mmcsd_core.o
obj-$(CONFIG_MMCSD_DISK) += mmcsd_disk.o
obj-$(CONFIG_MMCSD_SLOT) += mmcsd_slot.o

include $(TOPDIR)/Rules.make

mmcsd_core.o: $(mmcsd_core-objs)
	$(LD) -r -o $@ $(mmcsd_core-objs)

mmcsd_slot.o: $(mmcsd_slot-objs)
	$(LD) -r -o $@ $(mmcsd_slot-objs)
