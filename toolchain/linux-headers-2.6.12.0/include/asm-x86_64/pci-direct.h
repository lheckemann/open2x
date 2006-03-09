#ifndef ASM_PCI_DIRECT_H
#define ASM_PCI_DIRECT_H 1

#include <linux/types.h>
#include <asm/io.h>

/* Direct PCI access. This is used for PCI accesses in early boot before
   the PCI subsystem works. */ 

#define PDprintk(x...)

static inline __u32 read_pci_config(__u8 bus, __u8 slot, __u8 func, __u8 offset)
{
	__u32 v; 
	outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
	v = inl(0xcfc); 
	if (v != 0xffffffff)
		PDprintk("%x reading 4 from %x: %x\n", slot, offset, v);
	return v;
}

static inline __u8 read_pci_config_byte(__u8 bus, __u8 slot, __u8 func, __u8 offset)
{
	__u8 v; 
	outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
	v = inb(0xcfc + (offset&3)); 
	PDprintk("%x reading 1 from %x: %x\n", slot, offset, v);
	return v;
}

static inline __u16 read_pci_config_16(__u8 bus, __u8 slot, __u8 func, __u8 offset)
{
	__u16 v; 
	outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
	v = inw(0xcfc + (offset&2)); 
	PDprintk("%x reading 2 from %x: %x\n", slot, offset, v);
	return v;
}

static inline void write_pci_config(__u8 bus, __u8 slot, __u8 func, __u8 offset,
				    __u32 val)
{
	PDprintk("%x writing to %x: %x\n", slot, offset, val); 
	outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
	outl(val, 0xcfc); 
}

#endif
