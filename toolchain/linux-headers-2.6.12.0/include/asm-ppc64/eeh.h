/* 
 * eeh.h
 * Copyright (C) 2001  Dave Engebretsen & Todd Inglett IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _PPC64_EEH_H
#define _PPC64_EEH_H

#include <linux/init.h>

struct pci_dev;
struct device_node;

/* Values for eeh_mode bits in device_node */
#define EEH_MODE_SUPPORTED	(1<<0)
#define EEH_MODE_NOCHECK	(1<<1)
#define EEH_MODE_ISOLATED	(1<<2)

#ifdef CONFIG_PPC_PSERIES
extern void __init eeh_init(void);
unsigned long eeh_check_failure(const volatile void *token, unsigned long val);
int eeh_dn_check_failure (struct device_node *dn, struct pci_dev *dev);
void *eeh_ioremap(unsigned long addr, void *vaddr);
void __init pci_addr_cache_build(void);
#else
#define eeh_check_failure(token, val) (val)
#endif

/**
 * eeh_add_device_early
 * eeh_add_device_late
 *
 * Perform eeh initialization for devices added after boot.
 * Call eeh_add_device_early before doing any i/o to the
 * device (including config space i/o).  Call eeh_add_device_late
 * to finish the eeh setup for this device.
 */
void eeh_add_device_early(struct device_node *);
void eeh_add_device_late(struct pci_dev *);

/**
 * eeh_remove_device - undo EEH setup for the indicated pci device
 * @dev: pci device to be removed
 *
 * This routine should be when a device is removed from a running
 * system (e.g. by hotplug or dlpar).
 */
void eeh_remove_device(struct pci_dev *);

#define EEH_DISABLE		0
#define EEH_ENABLE		1
#define EEH_RELEASE_LOADSTORE	2
#define EEH_RELEASE_DMA		3

/**
 * Notifier event flags.
 */
#define EEH_NOTIFY_FREEZE  1

/** EEH event -- structure holding pci slot data that describes
 *  a change in the isolation status of a PCI slot.  A pointer
 *  to this struct is passed as the data pointer in a notify callback.
 */
struct eeh_event {
	struct list_head     list;
	struct pci_dev       *dev;
	struct device_node   *dn;
	int                  reset_state;
};

/** Register to find out about EEH events. */
int eeh_register_notifier(struct notifier_block *nb);
int eeh_unregister_notifier(struct notifier_block *nb);

/**
 * EEH_POSSIBLE_ERROR() -- test for possible MMIO failure.
 *
 * If this macro yields TRUE, the caller relays to eeh_check_failure()
 * which does further tests out of line.
 */
#define EEH_POSSIBLE_ERROR(val, type)	((val) == (type)~0)

/*
 * Reads from a device which has been isolated by EEH will return
 * all 1s.  This macro gives an all-1s value of the given size (in
 * bytes: 1, 2, or 4) for comparing with the result of a read.
 */
#define EEH_IO_ERROR_VALUE(size)	(~0U >> ((4 - (size)) * 8))

#else /* !CONFIG_EEH */
static inline void eeh_init(void) { }

static inline unsigned long eeh_check_failure(const volatile void *token, unsigned long val)
{
	return val;
}

static inline int eeh_dn_check_failure(struct device_node *dn, struct pci_dev *dev)
{
	return 0;
}

static inline void pci_addr_cache_build(void) { }

static inline void eeh_add_device_early(struct device_node *dn) { }

static inline void eeh_add_device_late(struct pci_dev *dev) { }

static inline void eeh_remove_device(struct pci_dev *dev) { }

#define EEH_POSSIBLE_ERROR(val, type) (0)
#define EEH_IO_ERROR_VALUE(size) (-1UL)
#endif /* CONFIG_EEH */

/* 
 * MMIO read/write operations with EEH support.
 */
static inline __u8 eeh_readb(const volatile void *addr)
{
	__u8 val = in_8(addr);
	if (EEH_POSSIBLE_ERROR(val, __u8))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_writeb(__u8 val, volatile void *addr)
{
	out_8(addr, val);
}

static inline __u16 eeh_readw(const volatile void *addr)
{
	__u16 val = in_le16(addr);
	if (EEH_POSSIBLE_ERROR(val, __u16))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_writew(__u16 val, volatile void *addr)
{
	out_le16(addr, val);
}
static inline __u16 eeh_raw_readw(const volatile void *addr)
{
	__u16 val = in_be16(addr);
	if (EEH_POSSIBLE_ERROR(val, __u16))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_raw_writew(__u16 val, volatile void *addr)
{
	out_be16(addr, val);
}

static inline __u32 eeh_readl(const volatile void *addr)
{
	__u32 val = in_le32(addr);
	if (EEH_POSSIBLE_ERROR(val, __u32))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_writel(__u32 val, volatile void *addr)
{
	out_le32(addr, val);
}
static inline __u32 eeh_raw_readl(const volatile void *addr)
{
	__u32 val = in_be32(addr);
	if (EEH_POSSIBLE_ERROR(val, __u32))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_raw_writel(__u32 val, volatile void *addr)
{
	out_be32(addr, val);
}

static inline __u64 eeh_readq(const volatile void *addr)
{
	__u64 val = in_le64(addr);
	if (EEH_POSSIBLE_ERROR(val, __u64))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_writeq(__u64 val, volatile void *addr)
{
	out_le64(addr, val);
}
static inline __u64 eeh_raw_readq(const volatile void *addr)
{
	__u64 val = in_be64(addr);
	if (EEH_POSSIBLE_ERROR(val, __u64))
		return eeh_check_failure(addr, val);
	return val;
}
static inline void eeh_raw_writeq(__u64 val, volatile void *addr)
{
	out_be64(addr, val);
}

#define EEH_CHECK_ALIGN(v,a) \
	((((unsigned long)(v)) & ((a) - 1)) == 0)

static inline void eeh_memset_io(volatile void *addr, int c, unsigned long n) {
	__u32 lc = c;
	lc |= lc << 8;
	lc |= lc << 16;

	while(n && !EEH_CHECK_ALIGN(addr, 4)) {
		*((volatile __u8 *)addr) = c;
		addr = (void *)((unsigned long)addr + 1);
		n--;
	}
	while(n >= 4) {
		*((volatile __u32 *)addr) = lc;
		addr = (void *)((unsigned long)addr + 4);
		n -= 4;
	}
	while(n) {
		*((volatile __u8 *)addr) = c;
		addr = (void *)((unsigned long)addr + 1);
		n--;
	}
	__asm__ __volatile__ ("sync" : : : "memory");
}
static inline void eeh_memcpy_fromio(void *dest, const volatile void *src, unsigned long n) {
	void *vsrc = (void *) src;
	void *destsave = dest;
	unsigned long nsave = n;

	while(n && (!EEH_CHECK_ALIGN(vsrc, 4) || !EEH_CHECK_ALIGN(dest, 4))) {
		*((__u8 *)dest) = *((volatile __u8 *)vsrc);
		__asm__ __volatile__ ("eieio" : : : "memory");
		vsrc = (void *)((unsigned long)vsrc + 1);
		dest = (void *)((unsigned long)dest + 1);			
		n--;
	}
	while(n > 4) {
		*((__u32 *)dest) = *((volatile __u32 *)vsrc);
		__asm__ __volatile__ ("eieio" : : : "memory");
		vsrc = (void *)((unsigned long)vsrc + 4);
		dest = (void *)((unsigned long)dest + 4);			
		n -= 4;
	}
	while(n) {
		*((__u8 *)dest) = *((volatile __u8 *)vsrc);
		__asm__ __volatile__ ("eieio" : : : "memory");
		vsrc = (void *)((unsigned long)vsrc + 1);
		dest = (void *)((unsigned long)dest + 1);			
		n--;
	}
	__asm__ __volatile__ ("sync" : : : "memory");

	/* Look for ffff's here at dest[n].  Assume that at least 4 bytes
	 * were copied. Check all four bytes.
	 */
	if ((nsave >= 4) &&
		(EEH_POSSIBLE_ERROR((*((__u32 *) destsave+nsave-4)), __u32))) {
		eeh_check_failure(src, (*((__u32 *) destsave+nsave-4)));
	}
}

static inline void eeh_memcpy_toio(volatile void *dest, const void *src,
				   unsigned long n)
{
	void *vdest = (void *) dest;

	while(n && (!EEH_CHECK_ALIGN(vdest, 4) || !EEH_CHECK_ALIGN(src, 4))) {
		*((volatile __u8 *)vdest) = *((__u8 *)src);
		src = (void *)((unsigned long)src + 1);
		vdest = (void *)((unsigned long)vdest + 1);			
		n--;
	}
	while(n > 4) {
		*((volatile __u32 *)vdest) = *((volatile __u32 *)src);
		src = (void *)((unsigned long)src + 4);
		vdest = (void *)((unsigned long)vdest + 4);			
		n-=4;
	}
	while(n) {
		*((volatile __u8 *)vdest) = *((__u8 *)src);
		src = (void *)((unsigned long)src + 1);
		vdest = (void *)((unsigned long)vdest + 1);			
		n--;
	}
	__asm__ __volatile__ ("sync" : : : "memory");
}

#undef EEH_CHECK_ALIGN

static inline __u8 eeh_inb(unsigned long port)
{
	__u8 val;
	if (!_IO_IS_VALID(port))
		return ~0;
	val = in_8((__u8 *)(port+pci_io_base));
	if (EEH_POSSIBLE_ERROR(val, __u8))
		return eeh_check_failure((void *)(port), val);
	return val;
}

static inline void eeh_outb(__u8 val, unsigned long port)
{
	if (_IO_IS_VALID(port))
		out_8((__u8 *)(port+pci_io_base), val);
}

static inline __u16 eeh_inw(unsigned long port)
{
	__u16 val;
	if (!_IO_IS_VALID(port))
		return ~0;
	val = in_le16((__u16 *)(port+pci_io_base));
	if (EEH_POSSIBLE_ERROR(val, __u16))
		return eeh_check_failure((void *)(port), val);
	return val;
}

static inline void eeh_outw(__u16 val, unsigned long port)
{
	if (_IO_IS_VALID(port))
		out_le16((__u16 *)(port+pci_io_base), val);
}

static inline __u32 eeh_inl(unsigned long port)
{
	__u32 val;
	if (!_IO_IS_VALID(port))
		return ~0;
	val = in_le32((__u32 *)(port+pci_io_base));
	if (EEH_POSSIBLE_ERROR(val, __u32))
		return eeh_check_failure((void *)(port), val);
	return val;
}

static inline void eeh_outl(__u32 val, unsigned long port)
{
	if (_IO_IS_VALID(port))
		out_le32((__u32 *)(port+pci_io_base), val);
}

/* in-string eeh macros */
static inline void eeh_insb(unsigned long port, void * buf, int ns)
{
	_insb((__u8 *)(port+pci_io_base), buf, ns);
	if (EEH_POSSIBLE_ERROR((*(((__u8*)buf)+ns-1)), __u8))
		eeh_check_failure((void *)(port), *(__u8*)buf);
}

static inline void eeh_insw_ns(unsigned long port, void * buf, int ns)
{
	_insw_ns((__u16 *)(port+pci_io_base), buf, ns);
	if (EEH_POSSIBLE_ERROR((*(((__u16*)buf)+ns-1)), __u16))
		eeh_check_failure((void *)(port), *(__u16*)buf);
}

static inline void eeh_insl_ns(unsigned long port, void * buf, int nl)
{
	_insl_ns((__u32 *)(port+pci_io_base), buf, nl);
	if (EEH_POSSIBLE_ERROR((*(((__u32*)buf)+nl-1)), __u32))
		eeh_check_failure((void *)(port), *(__u32*)buf);
}

#endif /* _PPC64_EEH_H */
