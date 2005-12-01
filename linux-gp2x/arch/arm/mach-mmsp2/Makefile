#
# Makefile for the linux kernel
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).

USE_STANDARD_AS_RULE := true

O_TARGET		:= mmsp2.o

# Common support (must be linked before board specific support)
obj-y := generic.o irq.o dma.o pcipool.o
obj-m :=
obj-n :=
obj-  :=

export-objs := generic.o irq.o mmsp2_usbh.o dma.o pcipool.o \
               usb_ctl.o usb_recv.o usb_send.o \
               proto_mlc.o proto_fdc.o proto_scale.o proto_dualcpu.o proto_gpio.o proto_pwrman.o proto_dpc.o \
			   proto_isp.o

obj-$(CONFIG_USB_OHCI_MMSP2) += mmsp2_usbh.o
obj-$(CONFIG_ARCH_MMSP2)     += proto_pwrman.o proto_dpc.o proto_mlc.o proto_fdc.o proto_scale.o proto_dualcpu.o proto_gpio.o proto_isp.o

# Spcific board support
obj-$(CONFIG_MACH_MMSP2_MDK) += mdk.o

list-multi += mmsp2usb_core.o
mmsp2usb_core-objs := usb_ctl.o usb_ep0.o usb_recv.o usb_send.o
obj-$(CONFIG_MMSP2_USBDEV) += mmsp2usb_core.o
obj-$(CONFIG_MMSP2_USBDEV_NETLINK) += usb-eth.o

include $(TOPDIR)/Rules.make

mmsp2usb_core.o: $(mmsp2usb_core-objs)
	$(LD) -r -o $@ $(mmsp2usb_core-objs)
