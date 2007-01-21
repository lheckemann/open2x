/*
 * linux/arch/arm/mach-mmsp2/usbh_pcibuf.c
 *
 * Copyright (C) 2004,2005 DIGNSYS Inc. (www.dignsys.com)
 * Kane Ahn < hbahn@dignsys.com >
 * hhsong < hhsong@dignsys.com >
 *
 * Special pci_{map/unmap/dma_sync}_* routines for USB Host of MMSP2 CPU.
 *
 * Based on linux/kernel/arch/arm/mach-sa1100/sa1111-pcibuf.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/pci.h>
#include <linux/list.h>

#include <asm/hardware.h>

#define MMSP2_USBH_FAKEPCI (void*)(0xffff1010)

//#define DS_DEBUG
#undef DS_DEBUG

#ifdef DS_DEBUG
#  define ds_printk(x...) printk(x)
#else
#  define ds_printk(fmt, args...)
#endif

struct safe_buffer {
	struct list_head node;

	/* original request */
	void		*ptr;
	size_t		size;
	int		direction;

	/* safe buffer info */
	struct pci_pool *pool;
	void		*safe;
	dma_addr_t	safe_dma_addr;
};

LIST_HEAD(safe_buffers);

#define SIZE_SMALL	1024
#define SIZE_LARGE	(16*1024)

static struct pci_pool *small_buffer_pool, *large_buffer_pool;

static int __init
create_safe_buffer_pools(void)
{
	small_buffer_pool = pci_pool_create("usbh_small_dma_buffer",
					    MMSP2_USBH_FAKEPCI,
					    SIZE_SMALL,
					    0 /* byte alignment */,
					    0 /* no page-crossing issues */,
					    SLAB_KERNEL);
	if (0 == small_buffer_pool) {
		printk(KERN_ERR
		       "usbh_pcibuf: could not allocate small pci pool\n");
		return -1;
	}

	large_buffer_pool = pci_pool_create("usbh_large_dma_buffer",
					    MMSP2_USBH_FAKEPCI,
					    SIZE_LARGE,
					    0 /* byte alignment */,
					    0 /* no page-crossing issues */,
					    SLAB_KERNEL);
	if (0 == large_buffer_pool) {
		printk(KERN_ERR
		       "usbh_pcibuf: could not allocate large pci pool\n");
		pci_pool_destroy(small_buffer_pool);
		small_buffer_pool = 0;
		return -1;
	}

	return 0;
}

static void __exit
destroy_safe_buffer_pools(void)
{
	if (small_buffer_pool)
		pci_pool_destroy(small_buffer_pool);
	if (large_buffer_pool)
		pci_pool_destroy(large_buffer_pool);

	small_buffer_pool = large_buffer_pool = 0;
}


/* allocate a 'safe' buffer and keep track of it */
static struct safe_buffer *
alloc_safe_buffer(void *ptr, size_t size, int direction)
{
	struct safe_buffer *buf;
	struct pci_pool *pool;
	void *safe;
	dma_addr_t safe_dma_addr;

	ds_printk("%s(ptr=%p, size=%d, direction=%d)\n",
		__func__, ptr, size, direction);

	buf = kmalloc(sizeof(struct safe_buffer), GFP_ATOMIC);
	if (buf == 0) {
		printk(KERN_WARNING "%s: kmalloc failed\n", __func__);
		return 0;
	}

	if (size <= SIZE_SMALL) {
		pool = small_buffer_pool;
		safe = pci_pool_alloc(pool, GFP_ATOMIC, &safe_dma_addr);
	} else if (size <= SIZE_LARGE) {
		pool = large_buffer_pool;
		safe = pci_pool_alloc(pool, GFP_ATOMIC, &safe_dma_addr);
	} else {
		printk(KERN_DEBUG
		       "sa111_pcibuf: resorting to pci_alloc_consistent\n");
		pool = 0;
		safe = pci_alloc_consistent(MMSP2_USBH_FAKEPCI, size,
					    &safe_dma_addr);
	}

	if (safe == 0) {
		printk(KERN_WARNING
		       "%s: could not alloc dma memory (size=%d)\n",
		       __func__, size);
		kfree(buf);
		return 0;
	}

	buf->ptr = ptr;
	buf->size = size;
	buf->direction = direction;
	buf->pool = pool;
	buf->safe = safe;
	buf->safe_dma_addr = safe_dma_addr;

	MOD_INC_USE_COUNT;
	list_add(&buf->node, &safe_buffers);

	return buf;
}

/* determine if a buffer is from our "safe" pool */
static struct safe_buffer *
find_safe_buffer(dma_addr_t safe_dma_addr)
{
	struct list_head *entry;

	list_for_each(entry, &safe_buffers) {
		struct safe_buffer *b =
			list_entry(entry, struct safe_buffer, node);

		if (b->safe_dma_addr == safe_dma_addr) {
			return b;
		}
	}

	return 0;
}

static void
free_safe_buffer(struct safe_buffer *buf)
{
	ds_printk("%s(buf=%p)\n", __func__, buf);

	list_del(&buf->node);

	if (buf->pool)
		pci_pool_free(buf->pool, buf->safe, buf->safe_dma_addr);
	else
		pci_free_consistent(MMSP2_USBH_FAKEPCI, buf->size, buf->safe,
				    buf->safe_dma_addr);
	kfree(buf);

	MOD_DEC_USE_COUNT;
}

static inline int
dma_range_is_safe(dma_addr_t addr, size_t size)
{
#if 0
	unsigned int physaddr = SA1111_DMA_ADDR((unsigned int) addr);

	/* Any address within one megabyte of the start of the target
         * bank will be OK.  This is an overly conservative test:
         * other addresses can be OK depending on the dram
         * configuration.  (See sa1111.c:sa1111_check_dma_bug() * for
         * details.)
	 *
	 * We take care to ensure the entire dma region is within
	 * the safe range.
	 */

	return ((physaddr + size - 1) < (1<<20));
#else
	return 1;
#endif
}

/*
 * see if a buffer address is in an 'unsafe' range.  if it is
 * allocate a 'safe' buffer and copy the unsafe buffer into it.
 * substitute the safe buffer for the unsafe one.
 * (basically move the buffer from an unsafe area to a safe one)
 */
dma_addr_t
usbh_map_single(struct pci_dev *hwdev, void *ptr, size_t size, int direction)
{
	unsigned long flags;
	dma_addr_t dma_addr;

	ds_printk("%s(ptr=%p,size=%d,dir=%x)\n",
	       __func__, ptr, size, direction);

	BUG_ON(direction == PCI_DMA_NONE);

	local_irq_save(flags);

	dma_addr = virt_to_bus(ptr);

	if (!dma_range_is_safe(dma_addr, size)) {
		struct safe_buffer *buf;

		buf = alloc_safe_buffer(ptr, size, direction);
		if (buf == 0) {
			printk(KERN_ERR
			       "%s: unable to map unsafe buffer %p!\n",
			       __func__, ptr);
			local_irq_restore(flags);
			return 0;
		}

		ds_printk("%s: unsafe buffer %p (phy=%p) mapped to %p (phy=%p)\n",
			__func__,
			buf->ptr, (void *) virt_to_bus(buf->ptr),
			buf->safe, (void *) buf->safe_dma_addr);

		if ((direction == PCI_DMA_TODEVICE) ||
		    (direction == PCI_DMA_BIDIRECTIONAL)) {
			ds_printk("%s: copy out from unsafe %p, to safe %p, size %d\n",
				__func__, ptr, buf->safe, size);
			memcpy(buf->safe, ptr, size);
		}
		consistent_sync(buf->safe, size, direction);

		dma_addr = buf->safe_dma_addr;
	} else {
		consistent_sync(ptr, size, direction);
	}

	local_irq_restore(flags);
	return dma_addr;
}

/*
 * see if a mapped address was really a "safe" buffer and if so, copy
 * the data from the safe buffer back to the unsafe buffer and free up
 * the safe buffer.  (basically return things back to the way they
 * should be)
 */

void
usbh_unmap_single(struct pci_dev *hwdev, dma_addr_t dma_addr, size_t size, int direction)
{
	unsigned long flags;
	struct safe_buffer *buf;

	ds_printk("%s(ptr=%p,size=%d,dir=%x)\n",
		__func__, (void *) dma_addr, size, direction);

	BUG_ON(direction == PCI_DMA_NONE);

	local_irq_save(flags);

	buf = find_safe_buffer(dma_addr);
	if (buf) {
		BUG_ON(buf->size != size);
		BUG_ON(buf->direction != direction);

		ds_printk("%s: unsafe buffer %p (phy=%p) mapped to %p (phy=%p)\n",
			__func__,
			buf->ptr, (void *) virt_to_bus(buf->ptr),
			buf->safe, (void *) buf->safe_dma_addr);

		if ((direction == PCI_DMA_FROMDEVICE) ||
		    (direction == PCI_DMA_BIDIRECTIONAL)) {
			ds_printk("%s: copy back from safe %p, to unsafe %p size %d\n",
				__func__, buf->safe, buf->ptr, size);
			memcpy(buf->ptr, buf->safe, size);
		}
		free_safe_buffer(buf);
	}

	local_irq_restore(flags);
}

int
usbh_map_sg(struct scatterlist *sg, int nents, int direction)
{
	BUG();			/* Not implemented. */

	return -1;
}

void
usbh_unmap_sg(struct scatterlist *sg, int nents, int direction)
{
	BUG();			/* Not implemented. */
}

void
usbh_dma_sync_single(dma_addr_t dma_addr, size_t size, int direction)
{
	unsigned long flags;
	struct safe_buffer *buf;

	ds_printk("%s(ptr=%p,size=%d,dir=%x)\n",
		__func__, (void *) dma_addr, size, direction);

	local_irq_save(flags);

	buf = find_safe_buffer(dma_addr);
	if (buf) {
		BUG_ON(buf->size != size);
		BUG_ON(buf->direction != direction);

		ds_printk("%s: unsafe buffer %p (phy=%p) mapped to %p (phy=%p)\n",
			__func__,
			buf->ptr, (void *) virt_to_bus(buf->ptr),
			buf->safe, (void *) buf->safe_dma_addr);

		switch (direction) {
		case PCI_DMA_FROMDEVICE:
			ds_printk("%s: copy back from safe %p, to unsafe %p size %d\n",
				__func__, buf->safe, buf->ptr, size);
			memcpy(buf->ptr, buf->safe, size);
			break;
		case PCI_DMA_TODEVICE:
			ds_printk("%s: copy out from unsafe %p, to safe %p, size %d\n",
				__func__,buf->ptr, buf->safe, size);
			memcpy(buf->safe, buf->ptr, size);
			break;
		case PCI_DMA_BIDIRECTIONAL:
			BUG();	/* is this allowed?  what does it mean? */
		default:
			BUG();
		}
		consistent_sync(buf->safe, size, direction);
	} else {
		consistent_sync(bus_to_virt(dma_addr), size, direction);
	}

	local_irq_restore(flags);
}

void
usbh_dma_sync_sg(struct scatterlist *sg, int nelems, int direction)
{
	BUG();			/* Not implemented. */
}

EXPORT_SYMBOL(usbh_map_single);
EXPORT_SYMBOL(usbh_unmap_single);
EXPORT_SYMBOL(usbh_map_sg);
EXPORT_SYMBOL(usbh_unmap_sg);
EXPORT_SYMBOL(usbh_dma_sync_single);
EXPORT_SYMBOL(usbh_dma_sync_sg);

/* **************************************** */

static int __init usbh_pcibuf_init(void)
{
	int ret;

	printk("%s: initializing MMSP2 USB host PCI-DMA workaround\n", __FILE__);

	ret = create_safe_buffer_pools();

	return ret;
}
module_init(usbh_pcibuf_init);

static void __exit usbh_pcibuf_exit(void)
{
	BUG_ON(!list_empty(&safe_buffers));

	destroy_safe_buffer_pools();
}
module_exit(usbh_pcibuf_exit);

MODULE_LICENSE("GPL");
