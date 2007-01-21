//--------------------------------------------------------------
// Module      : CF
// File        : cf.h
// Description : function description to access CF
// 
//--------------------------------------------------------------

#ifndef _CF_H_
#define _CF_H_

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/semaphore.h>
#include <asm/hardware.h>
#include <asm/types.h>

#define POL_MOD		// Polling Mode
#undef INT_MOD		// Initrrupt Mode
#define NO_CD_INT	// Card Detect Interrupt Disable

/*DEBUG*/
#define DEBUG 0
#if DEBUG
	#define DPRINTK(args...)        printk(args)
#else
	#define DPRINTK(args...)
#endif

char    *pData, buff[256];

typedef struct {
	char tuple_id;	// tuple id
	u8 *major_vers;	// major version
	u8 *minor_vers;	// minor version
	u8 *manu_id;	// manufactuer id
	u8 *pro_nm;	// product name
	u8 *pro_info1;	// product information1
	u8 *pro_info2;	// product information2
	u32 size;	// offset size
} cis_reg;

typedef struct {
	u16 heads;
	u16 tracks;
	u16 sectors;
	u8 model[40];
} cf_spec;
	
struct cf_slot {
	u8 id;			/* slot id(begins at 0) assigned by slot driver */
	u8 slot;		/* If true, it's a CF_CARD */
	u64 size; 		/* total size of card in bytes */
	u8 drive_no;		/* dirve number */
	u16 len;		/* read length */
	u32 base_addr;		/* base address */
	u32 base_addr_io;
	u8 io_mod;
	u8 readonly; 		/* If true, it's readonly */
	u32 stat;		/* Card Status */
	cis_reg cis;		/* CIS struct */
	cf_spec spec;
};

struct cf_notifier {
	void (*add)(struct cf_slot *slot);
	void (*remove)(struct cf_slot *slot);
	struct cf_notifier *next;
};

extern void register_cf_user(struct cf_notifier *new);
extern int unregister_cf_user(struct cf_notifier *old);

#endif	// _CF_H_
