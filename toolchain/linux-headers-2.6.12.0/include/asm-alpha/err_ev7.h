#ifndef __ALPHA_ERR_EV7_H
#define __ALPHA_ERR_EV7_H 1

/*
 * Data for el packet class PAL (14), type LOGOUT_FRAME (1)
 */
struct ev7_pal_logout_subpacket {
	__u32 mchk_code;
	__u32 subpacket_count;
	__u64 whami;
	__u64 rbox_whami;
	__u64 rbox_int;
	__u64 exc_addr;
	union el_timestamp timestamp;
	__u64 halt_code;
	__u64 reserved;
};

/*
 * Data for el packet class PAL (14), type EV7_PROCESSOR (4)
 */
struct ev7_pal_processor_subpacket {
	__u64 i_stat;
	__u64 dc_stat;
	__u64 c_addr;
	__u64 c_syndrome_1;
	__u64 c_syndrome_0;
	__u64 c_stat;
	__u64 c_sts;
	__u64 mm_stat;
	__u64 exc_addr;
	__u64 ier_cm;
	__u64 isum;
	__u64 pal_base;
	__u64 i_ctl;
	__u64 process_context;
	__u64 cbox_ctl;
	__u64 cbox_stp_ctl;
	__u64 cbox_acc_ctl;
	__u64 cbox_lcl_set;
	__u64 cbox_gbl_set;
	__u64 bbox_ctl;
	__u64 bbox_err_sts;
	__u64 bbox_err_idx;
	__u64 cbox_ddp_err_sts;
	__u64 bbox_dat_rmp;
	__u64 reserved[2];
};

/*
 * Data for el packet class PAL (14), type EV7_ZBOX (5)
 */
struct ev7_pal_zbox_subpacket {
	__u32 zbox0_dram_err_status_1;
	__u32 zbox0_dram_err_status_2;
	__u32 zbox0_dram_err_status_3;
	__u32 zbox0_dram_err_ctl;
	__u32 zbox0_dram_err_adr;
	__u32 zbox0_dift_timeout;
	__u32 zbox0_dram_mapper_ctl;
	__u32 zbox0_frc_err_adr;
	__u32 zbox0_dift_err_status;
	__u32 reserved1;
	__u32 zbox1_dram_err_status_1;
	__u32 zbox1_dram_err_status_2;
	__u32 zbox1_dram_err_status_3;
	__u32 zbox1_dram_err_ctl;
	__u32 zbox1_dram_err_adr;
	__u32 zbox1_dift_timeout;
	__u32 zbox1_dram_mapper_ctl;
	__u32 zbox1_frc_err_adr;
	__u32 zbox1_dift_err_status;
	__u32 reserved2;
	__u64 cbox_ctl;
	__u64 cbox_stp_ctl;
	__u64 zbox0_error_pa;
	__u64 zbox1_error_pa;
	__u64 zbox0_ored_syndrome;
	__u64 zbox1_ored_syndrome;
	__u64 reserved3[2];
};

/*
 * Data for el packet class PAL (14), type EV7_RBOX (6)
 */
struct ev7_pal_rbox_subpacket {
	__u64 rbox_cfg;
	__u64 rbox_n_cfg;
	__u64 rbox_s_cfg;
	__u64 rbox_e_cfg;
	__u64 rbox_w_cfg;
	__u64 rbox_n_err;
	__u64 rbox_s_err;
	__u64 rbox_e_err;
	__u64 rbox_w_err;
	__u64 rbox_io_cfg;
	__u64 rbox_io_err;
	__u64 rbox_l_err;
	__u64 rbox_whoami;
	__u64 rbox_imask;
	__u64 rbox_intq;
	__u64 rbox_int;
	__u64 reserved[2];
};

/*
 * Data for el packet class PAL (14), type EV7_IO (7)
 */
struct ev7_pal_io_one_port {
	__u64 pox_err_sum;
	__u64 pox_tlb_err;
	__u64 pox_spl_cmplt;
	__u64 pox_trans_sum;
	__u64 pox_first_err;
	__u64 pox_mult_err;
	__u64 pox_dm_source;
	__u64 pox_dm_dest;
	__u64 pox_dm_size;
	__u64 pox_dm_ctrl;
	__u64 reserved;
};

struct ev7_pal_io_subpacket {
	__u64 io_asic_rev;
	__u64 io_sys_rev;
	__u64 io7_uph;
	__u64 hpi_ctl;
	__u64 crd_ctl;
	__u64 hei_ctl;
	__u64 po7_error_sum;
	__u64 po7_uncrr_sym;
	__u64 po7_crrct_sym;
	__u64 po7_ugbge_sym;
	__u64 po7_err_pkt0;
	__u64 po7_err_pkt1;
	__u64 reserved[2];
	struct ev7_pal_io_one_port ports[4];
};

/*
 * Environmental subpacket. Data used for el packets:
 * 	   class PAL (14), type AMBIENT_TEMPERATURE (10)
 * 	   class PAL (14), type AIRMOVER_FAN (11)
 * 	   class PAL (14), type VOLTAGE (12)
 * 	   class PAL (14), type INTRUSION (13)
 *	   class PAL (14), type POWER_SUPPLY (14)
 *	   class PAL (14), type LAN (15)
 *	   class PAL (14), type HOT_PLUG (16)
 */
struct ev7_pal_environmental_subpacket {
	__u16 cabinet;
	__u16 drawer;
	__u16 reserved1[2];
	__u8 module_type;
	__u8 unit_id;		/* unit reporting condition */
	__u8 reserved2;
	__u8 condition;		/* condition reported       */
};

/*
 * Convert environmental type to index
 */
static inline int ev7_lf_env_index(int type)
{
	BUG_ON((type < EL_TYPE__PAL__ENV__AMBIENT_TEMPERATURE) 
	       || (type > EL_TYPE__PAL__ENV__HOT_PLUG));

	return type - EL_TYPE__PAL__ENV__AMBIENT_TEMPERATURE;
}

/*
 * Data for generic el packet class PAL.
 */
struct ev7_pal_subpacket {
	union {
		struct ev7_pal_logout_subpacket logout;	     /* Type     1 */
		struct ev7_pal_processor_subpacket ev7;	     /* Type     4 */
		struct ev7_pal_zbox_subpacket zbox;	     /* Type     5 */
		struct ev7_pal_rbox_subpacket rbox;	     /* Type     6 */
		struct ev7_pal_io_subpacket io;		     /* Type     7 */
		struct ev7_pal_environmental_subpacket env;  /* Type 10-16 */
		__u64 as_quad[1];				     /* Raw __u64    */
	} by_type;
};

/*
 * Struct to contain collected logout from subpackets.
 */
struct ev7_lf_subpackets {
	struct ev7_pal_logout_subpacket *logout;		/* Type  1 */
	struct ev7_pal_processor_subpacket *ev7;		/* Type  4 */
	struct ev7_pal_zbox_subpacket *zbox;			/* Type  5 */
	struct ev7_pal_rbox_subpacket *rbox;			/* Type  6 */
	struct ev7_pal_io_subpacket *io;			/* Type  7 */
	struct ev7_pal_environmental_subpacket *env[7];	     /* Type 10-16 */

	unsigned int io_pid;
};

#endif /* __ALPHA_ERR_EV7_H */


