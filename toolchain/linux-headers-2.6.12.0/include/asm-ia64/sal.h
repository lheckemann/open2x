#ifndef _ASM_IA64_SAL_H
#define _ASM_IA64_SAL_H

/*
 * System Abstraction Layer definitions.
 *
 * This is based on version 2.5 of the manual "IA-64 System
 * Abstraction Layer".
 *
 * Copyright (C) 2001 Intel
 * Copyright (C) 2002 Jenna Hall <jenna.s.hall@intel.com>
 * Copyright (C) 2001 Fred Lewis <frederick.v.lewis@intel.com>
 * Copyright (C) 1998, 1999, 2001, 2003 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 * Copyright (C) 1999 Srinivasa Prasad Thirumalachar <sprasad@sprasad.engr.sgi.com>
 *
 * 02/01/04 J. Hall Updated Error Record Structures to conform to July 2001
 *		    revision of the SAL spec.
 * 01/01/03 fvlewis Updated Error Record Structures to conform with Nov. 2000
 *                  revision of the SAL spec.
 * 99/09/29 davidm	Updated for SAL 2.6.
 * 00/03/29 cfleck      Updated SAL Error Logging info for processor (SAL 2.6)
 *                      (plus examples of platform error info structures from smariset @ Intel)
 */

#define IA64_SAL_PLATFORM_FEATURE_BUS_LOCK_BIT		0
#define IA64_SAL_PLATFORM_FEATURE_IRQ_REDIR_HINT_BIT	1
#define IA64_SAL_PLATFORM_FEATURE_IPI_REDIR_HINT_BIT	2
#define IA64_SAL_PLATFORM_FEATURE_ITC_DRIFT_BIT	 	3

#define IA64_SAL_PLATFORM_FEATURE_BUS_LOCK	  (1<<IA64_SAL_PLATFORM_FEATURE_BUS_LOCK_BIT)
#define IA64_SAL_PLATFORM_FEATURE_IRQ_REDIR_HINT (1<<IA64_SAL_PLATFORM_FEATURE_IRQ_REDIR_HINT_BIT)
#define IA64_SAL_PLATFORM_FEATURE_IPI_REDIR_HINT (1<<IA64_SAL_PLATFORM_FEATURE_IPI_REDIR_HINT_BIT)
#define IA64_SAL_PLATFORM_FEATURE_ITC_DRIFT	  (1<<IA64_SAL_PLATFORM_FEATURE_ITC_DRIFT_BIT)

#ifndef __ASSEMBLY__

#include <linux/bcd.h>
#include <linux/efi.h>

#include <asm/pal.h>
#include <asm/system.h>
#include <asm/fpu.h>

extern spinlock_t sal_lock;

/* SAL spec _requires_ eight args for each call. */
#define __SAL_CALL(result,a0,a1,a2,a3,a4,a5,a6,a7)	\
	result = (*ia64_sal)(a0,a1,a2,a3,a4,a5,a6,a7)

# define SAL_CALL(result,args...) do {				\
	unsigned long __ia64_sc_flags;				\
	struct ia64_fpreg __ia64_sc_fr[6];			\
	ia64_save_scratch_fpregs(__ia64_sc_fr);			\
	spin_lock_irqsave(&sal_lock, __ia64_sc_flags);		\
	__SAL_CALL(result, args);				\
	spin_unlock_irqrestore(&sal_lock, __ia64_sc_flags);	\
	ia64_load_scratch_fpregs(__ia64_sc_fr);			\
} while (0)

# define SAL_CALL_NOLOCK(result,args...) do {		\
	unsigned long __ia64_scn_flags;			\
	struct ia64_fpreg __ia64_scn_fr[6];		\
	ia64_save_scratch_fpregs(__ia64_scn_fr);	\
	local_irq_save(__ia64_scn_flags);		\
	__SAL_CALL(result, args);			\
	local_irq_restore(__ia64_scn_flags);		\
	ia64_load_scratch_fpregs(__ia64_scn_fr);	\
} while (0)

# define SAL_CALL_REENTRANT(result,args...) do {	\
	struct ia64_fpreg __ia64_scs_fr[6];		\
	ia64_save_scratch_fpregs(__ia64_scs_fr);	\
	preempt_disable();				\
	__SAL_CALL(result, args);			\
	preempt_enable();				\
	ia64_load_scratch_fpregs(__ia64_scs_fr);	\
} while (0)

#define SAL_SET_VECTORS			0x01000000
#define SAL_GET_STATE_INFO		0x01000001
#define SAL_GET_STATE_INFO_SIZE		0x01000002
#define SAL_CLEAR_STATE_INFO		0x01000003
#define SAL_MC_RENDEZ			0x01000004
#define SAL_MC_SET_PARAMS		0x01000005
#define SAL_REGISTER_PHYSICAL_ADDR	0x01000006

#define SAL_CACHE_FLUSH			0x01000008
#define SAL_CACHE_INIT			0x01000009
#define SAL_PCI_CONFIG_READ		0x01000010
#define SAL_PCI_CONFIG_WRITE		0x01000011
#define SAL_FREQ_BASE			0x01000012
#define SAL_PHYSICAL_ID_INFO		0x01000013

#define SAL_UPDATE_PAL			0x01000020

struct ia64_sal_retval {
	/*
	 * A zero status value indicates call completed without error.
	 * A negative status value indicates reason of call failure.
	 * A positive status value indicates success but an
	 * informational value should be printed (e.g., "reboot for
	 * change to take effect").
	 */
	__s64 status;
	__u64 v0;
	__u64 v1;
	__u64 v2;
};

typedef struct ia64_sal_retval (*ia64_sal_handler) (__u64, ...);

enum {
	SAL_FREQ_BASE_PLATFORM = 0,
	SAL_FREQ_BASE_INTERVAL_TIMER = 1,
	SAL_FREQ_BASE_REALTIME_CLOCK = 2
};

/*
 * The SAL system table is followed by a variable number of variable
 * length descriptors.  The structure of these descriptors follows
 * below.
 * The defininition follows SAL specs from July 2000
 */
struct ia64_sal_systab {
	__u8 signature[4];	/* should be "SST_" */
	__u32 size;		/* size of this table in bytes */
	__u8 sal_rev_minor;
	__u8 sal_rev_major;
	__u16 entry_count;	/* # of entries in variable portion */
	__u8 checksum;
	__u8 reserved1[7];
	__u8 sal_a_rev_minor;
	__u8 sal_a_rev_major;
	__u8 sal_b_rev_minor;
	__u8 sal_b_rev_major;
	/* oem_id & product_id: terminating NUL is missing if string is exactly 32 bytes long. */
	__u8 oem_id[32];
	__u8 product_id[32];	/* ASCII product id  */
	__u8 reserved2[8];
};

enum sal_systab_entry_type {
	SAL_DESC_ENTRY_POINT = 0,
	SAL_DESC_MEMORY = 1,
	SAL_DESC_PLATFORM_FEATURE = 2,
	SAL_DESC_TR = 3,
	SAL_DESC_PTC = 4,
	SAL_DESC_AP_WAKEUP = 5
};

/*
 * Entry type:	Size:
 *	0	48
 *	1	32
 *	2	16
 *	3	32
 *	4	16
 *	5	16
 */
#define SAL_DESC_SIZE(type)	"\060\040\020\040\020\020"[(unsigned) type]

typedef struct ia64_sal_desc_entry_point {
	__u8 type;
	__u8 reserved1[7];
	__u64 pal_proc;
	__u64 sal_proc;
	__u64 gp;
	__u8 reserved2[16];
}ia64_sal_desc_entry_point_t;

typedef struct ia64_sal_desc_memory {
	__u8 type;
	__u8 used_by_sal;	/* needs to be mapped for SAL? */
	__u8 mem_attr;		/* current memory attribute setting */
	__u8 access_rights;	/* access rights set up by SAL */
	__u8 mem_attr_mask;	/* mask of supported memory attributes */
	__u8 reserved1;
	__u8 mem_type;		/* memory type */
	__u8 mem_usage;		/* memory usage */
	__u64 addr;		/* physical address of memory */
	__u32 length;	/* length (multiple of 4KB pages) */
	__u32 reserved2;
	__u8 oem_reserved[8];
} ia64_sal_desc_memory_t;

typedef struct ia64_sal_desc_platform_feature {
	__u8 type;
	__u8 feature_mask;
	__u8 reserved1[14];
} ia64_sal_desc_platform_feature_t;

typedef struct ia64_sal_desc_tr {
	__u8 type;
	__u8 tr_type;		/* 0 == instruction, 1 == data */
	__u8 regnum;		/* translation register number */
	__u8 reserved1[5];
	__u64 addr;		/* virtual address of area covered */
	__u64 page_size;		/* encoded page size */
	__u8 reserved2[8];
} ia64_sal_desc_tr_t;

typedef struct ia64_sal_desc_ptc {
	__u8 type;
	__u8 reserved1[3];
	__u32 num_domains;	/* # of coherence domains */
	__u64 domain_info;	/* physical address of domain info table */
} ia64_sal_desc_ptc_t;

typedef struct ia64_sal_ptc_domain_info {
	__u64 proc_count;		/* number of processors in domain */
	__u64 proc_list;		/* physical address of LID array */
} ia64_sal_ptc_domain_info_t;

typedef struct ia64_sal_ptc_domain_proc_entry {
	__u64 id  : 8;		/* id of processor */
	__u64 eid : 8;		/* eid of processor */
} ia64_sal_ptc_domain_proc_entry_t;


#define IA64_SAL_AP_EXTERNAL_INT 0

typedef struct ia64_sal_desc_ap_wakeup {
	__u8 type;
	__u8 mechanism;		/* 0 == external interrupt */
	__u8 reserved1[6];
	__u64 vector;		/* interrupt vector in range 0x10-0xff */
} ia64_sal_desc_ap_wakeup_t ;

extern ia64_sal_handler ia64_sal;
extern struct ia64_sal_desc_ptc *ia64_ptc_domain_info;

extern unsigned short sal_revision;	/* supported SAL spec revision */
extern unsigned short sal_version;	/* SAL version; OEM dependent */
#define SAL_VERSION_CODE(major, minor) ((BIN2BCD(major) << 8) | BIN2BCD(minor))

extern const char *ia64_sal_strerror (long status);
extern void ia64_sal_init (struct ia64_sal_systab *sal_systab);

/* SAL information type encodings */
enum {
	SAL_INFO_TYPE_MCA  = 0,		/* Machine check abort information */
        SAL_INFO_TYPE_INIT = 1,		/* Init information */
        SAL_INFO_TYPE_CMC  = 2,		/* Corrected machine check information */
        SAL_INFO_TYPE_CPE  = 3		/* Corrected platform error information */
};

/* Encodings for machine check parameter types */
enum {
	SAL_MC_PARAM_RENDEZ_INT    = 1,	/* Rendezvous interrupt */
	SAL_MC_PARAM_RENDEZ_WAKEUP = 2,	/* Wakeup */
	SAL_MC_PARAM_CPE_INT	   = 3	/* Corrected Platform Error Int */
};

/* Encodings for rendezvous mechanisms */
enum {
	SAL_MC_PARAM_MECHANISM_INT = 1,	/* Use interrupt */
	SAL_MC_PARAM_MECHANISM_MEM = 2	/* Use memory synchronization variable*/
};

/* Encodings for vectors which can be registered by the OS with SAL */
enum {
	SAL_VECTOR_OS_MCA	  = 0,
	SAL_VECTOR_OS_INIT	  = 1,
	SAL_VECTOR_OS_BOOT_RENDEZ = 2
};

/* Encodings for mca_opt parameter sent to SAL_MC_SET_PARAMS */
#define	SAL_MC_PARAM_RZ_ALWAYS		0x1
#define	SAL_MC_PARAM_BINIT_ESCALATE	0x10

/*
 * Definition of the SAL Error Log from the SAL spec
 */

/* SAL Error Record Section GUID Definitions */
#define SAL_PROC_DEV_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf1, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_MEM_DEV_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf2, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_SEL_DEV_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf3, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_PCI_BUS_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf4, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_SMBIOS_DEV_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf5, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_PCI_COMP_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf6, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_SPECIFIC_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf7, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_HOST_CTLR_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf8, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)
#define SAL_PLAT_BUS_ERR_SECT_GUID  \
    EFI_GUID(0xe429faf9, 0x3cb7, 0x11d4, 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81)

#define MAX_CACHE_ERRORS	6
#define MAX_TLB_ERRORS		6
#define MAX_BUS_ERRORS		1

/* Definition of version  according to SAL spec for logging purposes */
typedef struct sal_log_revision {
	__u8 minor;		/* BCD (0..99) */
	__u8 major;		/* BCD (0..99) */
} sal_log_revision_t;

/* Definition of timestamp according to SAL spec for logging purposes */
typedef struct sal_log_timestamp {
	__u8 slh_second;		/* Second (0..59) */
	__u8 slh_minute;		/* Minute (0..59) */
	__u8 slh_hour;		/* Hour (0..23) */
	__u8 slh_reserved;
	__u8 slh_day;		/* Day (1..31) */
	__u8 slh_month;		/* Month (1..12) */
	__u8 slh_year;		/* Year (00..99) */
	__u8 slh_century;		/* Century (19, 20, 21, ...) */
} sal_log_timestamp_t;

/* Definition of log record  header structures */
typedef struct sal_log_record_header {
	__u64 id;				/* Unique monotonically increasing ID */
	sal_log_revision_t revision;	/* Major and Minor revision of header */
	__u16 severity;			/* Error Severity */
	__u32 len;			/* Length of this error log in bytes */
	sal_log_timestamp_t timestamp;	/* Timestamp */
	efi_guid_t platform_guid;	/* Unique OEM Platform ID */
} sal_log_record_header_t;

#define sal_log_severity_recoverable	0
#define sal_log_severity_fatal		1
#define sal_log_severity_corrected	2

/* Definition of log section header structures */
typedef struct sal_log_sec_header {
    efi_guid_t guid;			/* Unique Section ID */
    sal_log_revision_t revision;	/* Major and Minor revision of Section */
    __u16 reserved;
    __u32 len;				/* Section length */
} sal_log_section_hdr_t;

typedef struct sal_log_mod_error_info {
	struct {
		__u64 check_info              : 1,
		    requestor_identifier    : 1,
		    responder_identifier    : 1,
		    target_identifier       : 1,
		    precise_ip              : 1,
		    reserved                : 59;
	} valid;
	__u64 check_info;
	__u64 requestor_identifier;
	__u64 responder_identifier;
	__u64 target_identifier;
	__u64 precise_ip;
} sal_log_mod_error_info_t;

typedef struct sal_processor_static_info {
	struct {
		__u64 minstate        : 1,
		    br              : 1,
		    cr              : 1,
		    ar              : 1,
		    rr              : 1,
		    fr              : 1,
		    reserved        : 58;
	} valid;
	pal_min_state_area_t min_state_area;
	__u64 br[8];
	__u64 cr[128];
	__u64 ar[128];
	__u64 rr[8];
	struct ia64_fpreg __attribute__ ((packed)) fr[128];
} sal_processor_static_info_t;

struct sal_cpuid_info {
	__u64 regs[5];
	__u64 reserved;
};

typedef struct sal_log_processor_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 proc_error_map      : 1,
		    proc_state_param    : 1,
		    proc_cr_lid         : 1,
		    psi_static_struct   : 1,
		    num_cache_check     : 4,
		    num_tlb_check       : 4,
		    num_bus_check       : 4,
		    num_reg_file_check  : 4,
		    num_ms_check        : 4,
		    cpuid_info          : 1,
		    reserved1           : 39;
	} valid;
	__u64 proc_error_map;
	__u64 proc_state_parameter;
	__u64 proc_cr_lid;
	/*
	 * The rest of this structure consists of variable-length arrays, which can't be
	 * expressed in C.
	 */
	sal_log_mod_error_info_t info[0];
	/*
	 * This is what the rest looked like if C supported variable-length arrays:
	 *
	 * sal_log_mod_error_info_t cache_check_info[.valid.num_cache_check];
	 * sal_log_mod_error_info_t tlb_check_info[.valid.num_tlb_check];
	 * sal_log_mod_error_info_t bus_check_info[.valid.num_bus_check];
	 * sal_log_mod_error_info_t reg_file_check_info[.valid.num_reg_file_check];
	 * sal_log_mod_error_info_t ms_check_info[.valid.num_ms_check];
	 * struct sal_cpuid_info cpuid_info;
	 * sal_processor_static_info_t processor_static_info;
	 */
} sal_log_processor_info_t;

/* Given a sal_log_processor_info_t pointer, return a pointer to the processor_static_info: */
#define SAL_LPI_PSI_INFO(l)									\
({	sal_log_processor_info_t *_l = (l);							\
	((sal_processor_static_info_t *)							\
	 ((char *) _l->info + ((_l->valid.num_cache_check + _l->valid.num_tlb_check		\
				+ _l->valid.num_bus_check + _l->valid.num_reg_file_check	\
				+ _l->valid.num_ms_check) * sizeof(sal_log_mod_error_info_t)	\
			       + sizeof(struct sal_cpuid_info))));				\
})

/* platform error log structures */

typedef struct sal_log_mem_dev_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 error_status    : 1,
		    physical_addr   : 1,
		    addr_mask       : 1,
		    node            : 1,
		    card            : 1,
		    module          : 1,
		    bank            : 1,
		    device          : 1,
		    row             : 1,
		    column          : 1,
		    bit_position    : 1,
		    requestor_id    : 1,
		    responder_id    : 1,
		    target_id       : 1,
		    bus_spec_data   : 1,
		    oem_id          : 1,
		    oem_data        : 1,
		    reserved        : 47;
	} valid;
	__u64 error_status;
	__u64 physical_addr;
	__u64 addr_mask;
	__u16 node;
	__u16 card;
	__u16 module;
	__u16 bank;
	__u16 device;
	__u16 row;
	__u16 column;
	__u16 bit_position;
	__u64 requestor_id;
	__u64 responder_id;
	__u64 target_id;
	__u64 bus_spec_data;
	__u8 oem_id[16];
	__u8 oem_data[1];			/* Variable length data */
} sal_log_mem_dev_err_info_t;

typedef struct sal_log_sel_dev_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 record_id       : 1,
		    record_type     : 1,
		    generator_id    : 1,
		    evm_rev         : 1,
		    sensor_type     : 1,
		    sensor_num      : 1,
		    event_dir       : 1,
		    event_data1     : 1,
		    event_data2     : 1,
		    event_data3     : 1,
		    reserved        : 54;
	} valid;
	__u16 record_id;
	__u8 record_type;
	__u8 timestamp[4];
	__u16 generator_id;
	__u8 evm_rev;
	__u8 sensor_type;
	__u8 sensor_num;
	__u8 event_dir;
	__u8 event_data1;
	__u8 event_data2;
	__u8 event_data3;
} sal_log_sel_dev_err_info_t;

typedef struct sal_log_pci_bus_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 err_status      : 1,
		    err_type        : 1,
		    bus_id          : 1,
		    bus_address     : 1,
		    bus_data        : 1,
		    bus_cmd         : 1,
		    requestor_id    : 1,
		    responder_id    : 1,
		    target_id       : 1,
		    oem_data        : 1,
		    reserved        : 54;
	} valid;
	__u64 err_status;
	__u16 err_type;
	__u16 bus_id;
	__u32 reserved;
	__u64 bus_address;
	__u64 bus_data;
	__u64 bus_cmd;
	__u64 requestor_id;
	__u64 responder_id;
	__u64 target_id;
	__u8 oem_data[1];			/* Variable length data */
} sal_log_pci_bus_err_info_t;

typedef struct sal_log_smbios_dev_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 event_type      : 1,
		    length          : 1,
		    time_stamp      : 1,
		    data            : 1,
		    reserved1       : 60;
	} valid;
	__u8 event_type;
	__u8 length;
	__u8 time_stamp[6];
	__u8 data[1];			/* data of variable length, length == slsmb_length */
} sal_log_smbios_dev_err_info_t;

typedef struct sal_log_pci_comp_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 err_status      : 1,
		    comp_info       : 1,
		    num_mem_regs    : 1,
		    num_io_regs     : 1,
		    reg_data_pairs  : 1,
		    oem_data        : 1,
		    reserved        : 58;
	} valid;
	__u64 err_status;
	struct {
		__u16 vendor_id;
		__u16 device_id;
		__u8 class_code[3];
		__u8 func_num;
		__u8 dev_num;
		__u8 bus_num;
		__u8 seg_num;
		__u8 reserved[5];
	} comp_info;
	__u32 num_mem_regs;
	__u32 num_io_regs;
	__u64 reg_data_pairs[1];
	/*
	 * array of address/data register pairs is num_mem_regs + num_io_regs elements
	 * long.  Each array element consists of a __u64 address followed by a __u64 data
	 * value.  The oem_data array immediately follows the reg_data_pairs array
	 */
	__u8 oem_data[1];			/* Variable length data */
} sal_log_pci_comp_err_info_t;

typedef struct sal_log_plat_specific_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 err_status      : 1,
		    guid            : 1,
		    oem_data        : 1,
		    reserved        : 61;
	} valid;
	__u64 err_status;
	efi_guid_t guid;
	__u8 oem_data[1];			/* platform specific variable length data */
} sal_log_plat_specific_err_info_t;

typedef struct sal_log_host_ctlr_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 err_status      : 1,
		    requestor_id    : 1,
		    responder_id    : 1,
		    target_id       : 1,
		    bus_spec_data   : 1,
		    oem_data        : 1,
		    reserved        : 58;
	} valid;
	__u64 err_status;
	__u64 requestor_id;
	__u64 responder_id;
	__u64 target_id;
	__u64 bus_spec_data;
	__u8 oem_data[1];			/* Variable length OEM data */
} sal_log_host_ctlr_err_info_t;

typedef struct sal_log_plat_bus_err_info {
	sal_log_section_hdr_t header;
	struct {
		__u64 err_status      : 1,
		    requestor_id    : 1,
		    responder_id    : 1,
		    target_id       : 1,
		    bus_spec_data   : 1,
		    oem_data        : 1,
		    reserved        : 58;
	} valid;
	__u64 err_status;
	__u64 requestor_id;
	__u64 responder_id;
	__u64 target_id;
	__u64 bus_spec_data;
	__u8 oem_data[1];			/* Variable length OEM data */
} sal_log_plat_bus_err_info_t;

/* Overall platform error section structure */
typedef union sal_log_platform_err_info {
	sal_log_mem_dev_err_info_t mem_dev_err;
	sal_log_sel_dev_err_info_t sel_dev_err;
	sal_log_pci_bus_err_info_t pci_bus_err;
	sal_log_smbios_dev_err_info_t smbios_dev_err;
	sal_log_pci_comp_err_info_t pci_comp_err;
	sal_log_plat_specific_err_info_t plat_specific_err;
	sal_log_host_ctlr_err_info_t host_ctlr_err;
	sal_log_plat_bus_err_info_t plat_bus_err;
} sal_log_platform_err_info_t;

/* SAL log over-all, multi-section error record structure (processor+platform) */
typedef struct err_rec {
	sal_log_record_header_t sal_elog_header;
	sal_log_processor_info_t proc_err;
	sal_log_platform_err_info_t plat_err;
	__u8 oem_data_pad[1024];
} ia64_err_rec_t;

/*
 * Now define a couple of inline functions for improved type checking
 * and convenience.
 */
static inline long
ia64_sal_freq_base (unsigned long which, unsigned long *ticks_per_second,
		    unsigned long *drift_info)
{
	struct ia64_sal_retval isrv;

	SAL_CALL(isrv, SAL_FREQ_BASE, which, 0, 0, 0, 0, 0, 0);
	*ticks_per_second = isrv.v0;
	*drift_info = isrv.v1;
	return isrv.status;
}

/* Flush all the processor and platform level instruction and/or data caches */
static inline __s64
ia64_sal_cache_flush (__u64 cache_type)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_CACHE_FLUSH, cache_type, 0, 0, 0, 0, 0, 0);
	return isrv.status;
}


/* Initialize all the processor and platform level instruction and data caches */
static inline __s64
ia64_sal_cache_init (void)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_CACHE_INIT, 0, 0, 0, 0, 0, 0, 0);
	return isrv.status;
}

/*
 * Clear the processor and platform information logged by SAL with respect to the machine
 * state at the time of MCA's, INITs, CMCs, or CPEs.
 */
static inline __s64
ia64_sal_clear_state_info (__u64 sal_info_type)
{
	struct ia64_sal_retval isrv;
	SAL_CALL_REENTRANT(isrv, SAL_CLEAR_STATE_INFO, sal_info_type, 0,
	              0, 0, 0, 0, 0);
	return isrv.status;
}


/* Get the processor and platform information logged by SAL with respect to the machine
 * state at the time of the MCAs, INITs, CMCs, or CPEs.
 */
static inline __u64
ia64_sal_get_state_info (__u64 sal_info_type, __u64 *sal_info)
{
	struct ia64_sal_retval isrv;
	SAL_CALL_REENTRANT(isrv, SAL_GET_STATE_INFO, sal_info_type, 0,
	              sal_info, 0, 0, 0, 0);
	if (isrv.status)
		return 0;

	return isrv.v0;
}

/*
 * Get the maximum size of the information logged by SAL with respect to the machine state
 * at the time of MCAs, INITs, CMCs, or CPEs.
 */
static inline __u64
ia64_sal_get_state_info_size (__u64 sal_info_type)
{
	struct ia64_sal_retval isrv;
	SAL_CALL_REENTRANT(isrv, SAL_GET_STATE_INFO_SIZE, sal_info_type, 0,
	              0, 0, 0, 0, 0);
	if (isrv.status)
		return 0;
	return isrv.v0;
}

/*
 * Causes the processor to go into a spin loop within SAL where SAL awaits a wakeup from
 * the monarch processor.  Must not lock, because it will not return on any cpu until the
 * monarch processor sends a wake up.
 */
static inline __s64
ia64_sal_mc_rendez (void)
{
	struct ia64_sal_retval isrv;
	SAL_CALL_NOLOCK(isrv, SAL_MC_RENDEZ, 0, 0, 0, 0, 0, 0, 0);
	return isrv.status;
}

/*
 * Allow the OS to specify the interrupt number to be used by SAL to interrupt OS during
 * the machine check rendezvous sequence as well as the mechanism to wake up the
 * non-monarch processor at the end of machine check processing.
 * Returns the complete ia64_sal_retval because some calls return more than just a status
 * value.
 */
static inline struct ia64_sal_retval
ia64_sal_mc_set_params (__u64 param_type, __u64 i_or_m, __u64 i_or_m_val, __u64 timeout, __u64 rz_always)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_MC_SET_PARAMS, param_type, i_or_m, i_or_m_val,
		 timeout, rz_always, 0, 0);
	return isrv;
}

/* Read from PCI configuration space */
static inline __s64
ia64_sal_pci_config_read (__u64 pci_config_addr, int type, __u64 size, __u64 *value)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_PCI_CONFIG_READ, pci_config_addr, size, type, 0, 0, 0, 0);
	if (value)
		*value = isrv.v0;
	return isrv.status;
}

/* Write to PCI configuration space */
static inline __s64
ia64_sal_pci_config_write (__u64 pci_config_addr, int type, __u64 size, __u64 value)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_PCI_CONFIG_WRITE, pci_config_addr, size, value,
	         type, 0, 0, 0);
	return isrv.status;
}

/*
 * Register physical addresses of locations needed by SAL when SAL procedures are invoked
 * in virtual mode.
 */
static inline __s64
ia64_sal_register_physical_addr (__u64 phys_entry, __u64 phys_addr)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_REGISTER_PHYSICAL_ADDR, phys_entry, phys_addr,
	         0, 0, 0, 0, 0);
	return isrv.status;
}

/*
 * Register software dependent code locations within SAL. These locations are handlers or
 * entry points where SAL will pass control for the specified event. These event handlers
 * are for the bott rendezvous, MCAs and INIT scenarios.
 */
static inline __s64
ia64_sal_set_vectors (__u64 vector_type,
		      __u64 handler_addr1, __u64 gp1, __u64 handler_len1,
		      __u64 handler_addr2, __u64 gp2, __u64 handler_len2)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_SET_VECTORS, vector_type,
			handler_addr1, gp1, handler_len1,
			handler_addr2, gp2, handler_len2);

	return isrv.status;
}

/* Update the contents of PAL block in the non-volatile storage device */
static inline __s64
ia64_sal_update_pal (__u64 param_buf, __u64 scratch_buf, __u64 scratch_buf_size,
		     __u64 *error_code, __u64 *scratch_buf_size_needed)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_UPDATE_PAL, param_buf, scratch_buf, scratch_buf_size,
	         0, 0, 0, 0);
	if (error_code)
		*error_code = isrv.v0;
	if (scratch_buf_size_needed)
		*scratch_buf_size_needed = isrv.v1;
	return isrv.status;
}

/* Get physical processor die mapping in the platform. */
static inline __s64
ia64_sal_physical_id_info(__u16 *splid)
{
	struct ia64_sal_retval isrv;
	SAL_CALL(isrv, SAL_PHYSICAL_ID_INFO, 0, 0, 0, 0, 0, 0, 0);
	if (splid)
		*splid = isrv.v0;
	return isrv.status;
}

extern unsigned long sal_platform_features;

extern int (*salinfo_platform_oemdata)(const __u8 *, __u8 **, __u64 *);

struct sal_ret_values {
	long r8; long r9; long r10; long r11;
};

#define IA64_SAL_OEMFUNC_MIN		0x02000000
#define IA64_SAL_OEMFUNC_MAX		0x03ffffff

extern int ia64_sal_oemcall(struct ia64_sal_retval *, __u64, __u64, __u64, __u64, __u64,
			    __u64, __u64, __u64);
extern int ia64_sal_oemcall_nolock(struct ia64_sal_retval *, __u64, __u64, __u64,
				   __u64, __u64, __u64, __u64, __u64);
extern int ia64_sal_oemcall_reentrant(struct ia64_sal_retval *, __u64, __u64, __u64,
				      __u64, __u64, __u64, __u64, __u64);
#ifdef CONFIG_HOTPLUG_CPU
/*
 * System Abstraction Layer Specification
 * Section 3.2.5.1: OS_BOOT_RENDEZ to SAL return State.
 * Note: region regs are stored first in head.S _start. Hence they must
 * stay up front.
 */
struct sal_to_os_boot {
	__u64 rr[8];		/* Region Registers */
	__u64	br[6];		/* br0: return addr into SAL boot rendez routine */
	__u64 gr1;		/* SAL:GP */
	__u64 gr12;		/* SAL:SP */
	__u64 gr13;		/* SAL: Task Pointer */
	__u64 fpsr;
	__u64	pfs;
	__u64 rnat;
	__u64 unat;
	__u64 bspstore;
	__u64 dcr;		/* Default Control Register */
	__u64 iva;
	__u64 pta;
	__u64 itv;
	__u64 pmv;
	__u64 cmcv;
	__u64 lrr[2];
	__u64 gr[4];
	__u64 pr;			/* Predicate registers */
	__u64 lc;			/* Loop Count */
	struct ia64_fpreg fp[20];
};

/*
 * Global array allocated for NR_CPUS at boot time
 */
extern struct sal_to_os_boot sal_boot_rendez_state[NR_CPUS];

extern void ia64_jump_to_sal(struct sal_to_os_boot *);
#endif

extern void ia64_sal_handler_init(void *entry_point, void *gpval);

#endif /* __ASSEMBLY__ */

#endif /* _ASM_IA64_SAL_H */
