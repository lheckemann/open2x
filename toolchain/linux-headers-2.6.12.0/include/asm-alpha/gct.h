#ifndef __ALPHA_GCT_H
#define __ALPHA_GCT_H

typedef __u64 gct_id;
typedef __u64 gct6_handle;

typedef struct __gct6_node {
	__u8 type;	
	__u8 subtype;
	__u16 size;
	__u32 hd_extension;
	gct6_handle owner;
	gct6_handle active_user;
	gct_id id;
	__u64 flags;
	__u16 rev;
	__u16 change_counter;
	__u16 max_child;
	__u16 reserved1;
	gct6_handle saved_owner;
	gct6_handle affinity;
	gct6_handle parent;
	gct6_handle next;
	gct6_handle prev;
	gct6_handle child;
	__u64 fw_flags;
	__u64 os_usage;
	__u64 fru_id;
	__u32 checksum;
	__u32 magic;	/* 'GLXY' */
} gct6_node;

typedef struct {
	__u8 type;	
	__u8 subtype;
	void (*callout)(gct6_node *);
} gct6_search_struct;

#define GCT_NODE_MAGIC	  0x59584c47	/* 'GLXY' */

/* 
 * node types 
 */
#define GCT_TYPE_HOSE			0x0E

/*
 * node subtypes
 */
#define GCT_SUBTYPE_IO_PORT_MODULE	0x2C

#define GCT_NODE_PTR(off) ((gct6_node *)((char *)hwrpb + 		\
					 hwrpb->frut_offset + 		\
					 (gct6_handle)(off)))		\

int gct6_find_nodes(gct6_node *, gct6_search_struct *);

#endif /* __ALPHA_GCT_H */

