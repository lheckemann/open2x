#ifndef LINUX_KEXEC_H
#define LINUX_KEXEC_H

#if CONFIG_KEXEC
#include <linux/types.h>
#include <linux/list.h>
#include <asm/kexec.h>

/* 
 * This structure is used to hold the arguments that are used when loading
 * kernel binaries.
 */

typedef unsigned long kimage_entry_t;
#define IND_DESTINATION  0x1
#define IND_INDIRECTION  0x2
#define IND_DONE         0x4
#define IND_SOURCE       0x8

#define KEXEC_SEGMENT_MAX 8
struct kexec_segment {
	void *buf;
	size_t bufsz;
	void *mem;
	size_t memsz;
};

struct kimage {
	kimage_entry_t head;
	kimage_entry_t *entry;
	kimage_entry_t *last_entry;

	unsigned long destination;
	unsigned long offset;

	unsigned long start;
	//MV struct page *reboot_code_pages;
	struct page *control_code_page;

	unsigned long nr_segments;
	struct kexec_segment segment[KEXEC_SEGMENT_MAX+1];

	struct list_head dest_pages;
	struct list_head unuseable_pages;
};


/* kexec interface functions */
extern void machine_kexec(struct kimage *image);
extern asmlinkage long sys_kexec(unsigned long entry, long nr_segments, 
	struct kexec_segment *segments);
extern struct kimage *kexec_image;
#endif
#endif /* LINUX_KEXEC_H */
