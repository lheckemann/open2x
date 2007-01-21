/*
 *  linux/arch/arm/mm/proc-arm1020E.S: MMU functions for ARM1020E
 *
 *  Copyright (C) 2000 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * These are the low level assembler for performing cache and TLB
 * functions on the arm1020E.
 */
#include <linux/linkage.h>
#include <linux/config.h>
#include <asm/assembler.h>
#include <asm/constants.h>
#include <asm/procinfo.h>
#include <asm/hardware.h>

/*
 * This is the maximum size of an area which will be invalidated
 * using the single invalidate entry instructions.  Anything larger
 * than this, and we go for the whole cache.
 *
 * This value should be chosen such that we choose the cheapest
 * alternative.
 */
#define MAX_AREA_SIZE	32768

/*
 * the cache line size of the I and D cache
 */
#define DCACHELINESIZE	32
#define ICACHELINESIZE	32

/*
 * and the page size
 */
#define LOG2PAGESIZE	12	/* == 4096 Bytes */
#define PAGESIZE	(1 << LOG2PAGESIZE)

/*
 * create some useful conditional macro definitions
 * we often need to know if we are ((not dcache disable) and writethrough) or ((not dcache disable) and writeback)
 */
#ifdef  CONFIG_CPU_DCACHE_DISABLE
    #undef  CONFIG_CPU_DCACHE_WRITETHROUGH
    #undef  CONFIG_CPU_DCACHE_WRITEBACK
    #undef  CONFIG_CPU_DCACHE_ENABLE
#else
  #ifdef  CONFIG_CPU_DCACHE_WRITETHROUGH
    #undef  CONFIG_CPU_DCACHE_WRITEBACK
  #else
    #define CONFIG_CPU_DCACHE_WRITEBACK
  #endif
    #define CONFIG_CPU_DCACHE_ENABLE
#endif

#ifdef  CONFIG_CPU_ICACHE_DISABLE
    #undef  CONFIG_CPU_ICACHE_ENABLE
#else
    #define CONFIG_CPU_ICACHE_ENABLE
#endif

	.text

/*
 * cpu_arm1020E_data_abort()
 *
 * obtain information about current aborted instruction
 * Note: we read user space.  This means we might cause a data
 * abort here if the I-TLB and D-TLB aren't seeing the same
 * picture.  Unfortunately, this does happen.  We live with it.
 *
 *  r2 = address of aborted instruction
 *  r3 = cpsr
 *
 * Returns:
 *  r0 = address of abort
 *  r1 = FSR
 *  r3 = corrupted
 *  r4 = corrupted
 */
	.align	5
ENTRY(cpu_arm1020E_data_abort)
	mrc	p15, 0, r1, c5, c0, 0		@ get FSR
	mrc	p15, 0, r0, c6, c0, 0		@ get FAR
	tst	r3, #PSR_T_BIT
	ldrneh	r3, [r2]			@ read aborted thumb instruction
	ldreq	r3, [r2]			@ read aborted ARM instruction
	bic	r1, r1, #1 << 11 | 1 << 10	@ clear bits 11 and 10 of FSR
	movne	r3, r3, lsl #(21 - 12)		@ move thumb bit 11 to ARM bit 20
	tst	r3, #1 << 20			@ check write
	orreq	r1, r1, #1 << 11
	mov	pc, lr

/*
 * cpu_arm1020E_check_bugs()
 */
ENTRY(cpu_arm1020E_check_bugs)
	mrs	ip, cpsr
	bic	ip, ip, #F_BIT
	msr	cpsr, ip
	mov	pc, lr

/*
 * cpu_arm1020E_proc_init()
 */
ENTRY(cpu_arm1020E_proc_init)
	mov	pc, lr

/*
 * cpu_arm1020E_proc_fin()
 */
ENTRY(cpu_arm1020E_proc_fin)
	stmfd	sp!, {lr}
	mov	ip, #F_BIT | I_BIT | SVC_MODE
	msr	cpsr_c, ip
	bl	cpu_arm1020E_cache_clean_invalidate_all
	mrc	p15, 0, r0, c1, c0, 0		@ ctrl register
	bic	r0, r0, #0x1000 		@ ...i............
	bic	r0, r0, #0x000e 		@ ............wca.
	mcr	p15, 0, r0, c1, c0, 0		@ disable caches
	ldmfd	sp!, {pc}

/*
 * cpu_arm1020E_reset(loc)
 *
 * Perform a soft reset of the system.	Put the CPU into the
 * same state as it would be if it had been reset, and branch
 * to what would be the reset vector.
 *
 * loc: location to jump to for soft reset
 */
	.align	5
ENTRY(cpu_arm1020E_reset)
	mov	ip, #0
	mcr	p15, 0, ip, c7, c7, 0		@ invalidate I,D caches
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
	mcr	p15, 0, ip, c8, c7, 0		@ invalidate I & D TLBs
	mrc	p15, 0, ip, c1, c0, 0		@ ctrl register
	bic	ip, ip, #0x000f 		@ ............wcam
	bic	ip, ip, #0x1100 		@ ...i...s........
	mcr	p15, 0, ip, c1, c0, 0		@ ctrl register
	mov	pc, r0

/*
 * cpu_arm1020E_do_idle()
 */
	.align	5
ENTRY(cpu_arm1020E_do_idle)
	mcr	p15, 0, r0, c7, c0, 4		@ Wait for interrupt
	mov	pc, lr

/* ================================= CACHE ================================ */


/*
 * cpu_arm1020E_cache_clean_invalidate_all()
 *
 * clean and invalidate all cache lines
 *
 * Note:
 *  1. we should preserve r0 and ip at all times
 */
	.align	5
ENTRY(cpu_arm1020E_cache_clean_invalidate_all)
	mov	r2, #1
cpu_arm1020E_cache_clean_invalidate_all_r2:

#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mov	r1, #0x0F << 5			@ 16 segments
1:	orr	r3, r1, #63 << 26		@ 64 entries
2:	mcr	p15, 0, r3, c7, c14, 2		@ clean and invalidate D index
	subs	r3, r3, #1 << 26
	bcs	2b
	subs	r1, r1, #1 << 5
	bcs	1b				@ segments 15 to 0
#endif

	mov	r1, #0

#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, r1, c7, c6, 0		@ invalidate D cache
#endif

#ifdef CONFIG_CPU_ICACHE_ENABLE
	teq	r2, #0
	mcrne	p15, 0, r1, c7, c5, 0		@ invalidate I cache
#endif
	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm1020E_cache_clean_invalidate_range(start, end, flags)
 *
 * clean and invalidate all cache lines associated with this area of memory
 *
 * start: Area start address
 * end:   Area end address
 * flags: nonzero for I cache as well
 */
	.align	5
ENTRY(cpu_arm1020E_cache_clean_invalidate_range)
	bic	r0, r0, #DCACHELINESIZE - 1
	sub	r3, r1, r0
	cmp	r3, #MAX_AREA_SIZE
	bhs	cpu_arm1020E_cache_clean_invalidate_all_r2
1:
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#endif
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
#endif
#ifdef CONFIG_CPU_ICACHE_ENABLE
	teq	r2, #0
	mcrne	p15, 0, r0, c7, c5, 1		@ invalidate I entry
#endif
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	bls	1b				@ unsigned lower or same - must include end point (r1)!

	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm1020E_flush_ram_page(page)
 *
 * clean and invalidate all cache lines associated with this area of memory
 *
 * page: page to clean and invalidate
 */
	.align	5
ENTRY(cpu_arm1020E_flush_ram_page)
	mov	r1, #PAGESIZE
	mov	r0, r0, LSR #LOG2PAGESIZE	@ round down to nearest page
	mov	r0, r0, LSL #LOG2PAGESIZE
1:
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#endif
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
#endif
#ifdef CONFIG_CPU_ICACHE_ENABLE
	mcr	p15, 0, r0, c7, c5, 1		@ invalidate I entry
#endif
	add	r0, r0, #DCACHELINESIZE
	subs	r1, r1, #DCACHELINESIZE
	bne	1b

	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr

/* ================================ D-CACHE =============================== */

/*
 * cpu_arm1020E_dcache_invalidate_range(start, end)
 *
 * throw away all D-cached data in specified region without an obligation
 * to write them back.	Note however that we must clean the D-cached entries
 * around the boundaries if the start and/or end address are not cache
 * aligned.
 *
 * start: virtual start address
 * end:   virtual end address
 */
	.align	5
ENTRY(cpu_arm1020E_dcache_invalidate_range)
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	bic	r0, r0, #DCACHELINESIZE - 1
#endif
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	tst	r0, #DCACHELINESIZE - 1
	bic	r0, r0, #DCACHELINESIZE - 1
	mcrne	p15, 0, r0, c7, c10, 1		@ clean D entry at start
	tst	r1, #DCACHELINESIZE - 1
	mcrne	p15, 0, r1, c7, c10, 1		@ clean D entry at end
#endif

1:
#ifdef CONFIG_CPU_DCACHE_ENABLE
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#endif
#ifdef CONFIG_CPU_ICACHE_ENABLE
	mcr	p15, 0, r0, c7, c5, 1		@ invalidate I entry
#endif
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	bls	1b

	/* Even if the D cache is off still drain the write buffer */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ Drain write buffer
	mov	pc, lr

/*
 * cpu_arm1020E_dcache_clean_range(start, end)
 *
 * For the specified virtual address range, ensure that all caches contain
 * clean data, such that peripheral accesses to the physical RAM fetch
 * correct data.
 *
 * start: virtual start address
 * end:   virtual end address
 */
	.align	5
ENTRY(cpu_arm1020E_dcache_clean_range)

	mov	r2, #0

#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	bic	r0, r0, #DCACHELINESIZE - 1
	sub	r3, r1, r0
	cmp	r3, #MAX_AREA_SIZE
	bhs	cpu_arm1020E_cache_clean_invalidate_all_r2

1:	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	bls	1b
#endif

	mcr	p15, 0, r2, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm1020E_dcache_clean_page(page)
 *
 * Cleans a single page of dcache so that if we have any future aliased
 * mappings, they will be consistent at the time that they are created.
 *
 * page: virtual address of page to clean from dcache
 *
 * Note:
 *     we don't invalidate the entries since when we write the page
 *     out to disk, the entries may get reloaded into the cache.
 */
	.align	5
ENTRY(cpu_arm1020E_dcache_clean_page)
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mov	r0, r0, LSR #LOG2PAGESIZE	@ round down to nearest page
	mov	r0, r0, LSL #LOG2PAGESIZE
	mov	r1, #PAGESIZE
1:
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
	add	r0, r0, #DCACHELINESIZE
	subs	r1, r1, #DCACHELINESIZE
	bne	1b
#endif
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm1020E_dcache_clean_entry(addr)
 *
 * Clean the specified entry of any caches such that the MMU
 * translation fetches will obtain correct data.
 *
 * addr: cache-unaligned virtual address
 */
	.align	5
ENTRY(cpu_arm1020E_dcache_clean_entry)
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	bic	r0, r0, #DCACHELINESIZE - 1
	mcr	p15, 0, r0, c7, c10, 1		@ clean single D entry
#endif
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr

/* ================================ I-CACHE =============================== */

/*
 * cpu_arm1020E_icache_invalidate_range(start, end)
 *
 * invalidate a range of virtual addresses from the Icache
 *
 * This is a little misleading, it is not intended to clean out
 * the i-cache but to make sure that any data written to the
 * range is made consistent.  This means that when we execute code
 * in that region, everything works as we expect.
 *
 * This generally means writing back data in the Dcache and
 * write buffer and invalidating the Icache over that region
 *
 * start: virtual start address
 * end:   virtual end address
 *
 * NOTE: ICACHELINESIZE == DCACHELINESIZE (so we don't need to
 * loop twice, once for i-cache, once for d-cache)
 */
	.align	5
ENTRY(cpu_arm1020E_icache_invalidate_range)
	bic	r0, r0, #ICACHELINESIZE - 1
	sub	r3, r1, r0
	cmp	r3, #MAX_AREA_SIZE
	movhs	r2, #1
	bhs	cpu_arm1020E_cache_clean_invalidate_all_r2

1:
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mcr	p15, 0, r0, c7, c10, 1		@ Clean D entry
#endif
#ifdef CONFIG_CPU_ICACHE_ENABLE
	mcr	p15, 0, r0, c7, c5, 1		@ Invalidate I entry
#endif
	add	r0, r0, #DCACHELINESIZE
	cmp	r0, r1
	bls	1b				@ unsigned lower or same - includes r1 entry

	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mov	pc, lr

ENTRY(cpu_arm1020E_icache_invalidate_page)
	mov	r0, r0, LSR #LOG2PAGESIZE	@ round down to nearest page
	mov	r0, r0, LSL #LOG2PAGESIZE
	add	r1, r0, #PAGESIZE
	b	cpu_arm1020E_icache_invalidate_range

/* ================================== TLB ================================= */

/*
 * cpu_arm1020E_tlb_invalidate_all()
 *
 * Invalidate all TLB entries
 */
	.align	5
ENTRY(cpu_arm1020E_tlb_invalidate_all)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mcr	p15, 0, r0, c8, c7, 0		@ invalidate I & D tlbs
	mov	pc, lr

/*
 * cpu_arm1020E_tlb_invalidate_range(start, end)
 *
 * invalidate TLB entries covering the specified range
 *
 * start: range start address
 * end:   range end address
 */
	.align	5
ENTRY(cpu_arm1020E_tlb_invalidate_range)
	sub	r3, r1, r0
	cmp	r3, #256 * PAGESIZE
	bhs	cpu_arm1020E_tlb_invalidate_all
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 4		@ drain WB
	mov	r3, #PAGESIZE
	sub	r3, r3, #1
	bic	r0, r0, r3
1:	mcr	p15, 0, r0, c8, c6, 1		@ invalidate D TLB entry
	mcr	p15, 0, r0, c8, c5, 1		@ invalidate I TLB entry
	add	r0, r0, #PAGESIZE
	cmp	r0, r1
	bls	1b
	mov	pc, lr

/*
 * cpu_arm1020E_tlb_invalidate_page(page, flags)
 *
 * invalidate the TLB entries for the specified page.
 *
 * page:  page to invalidate
 * flags: non-zero if we include the I TLB
 */
	.align	5
ENTRY(cpu_arm1020E_tlb_invalidate_page)
	mov	r3, #0
	mcr	p15, 0, r3, c7, c10, 4		@ drain WB
	mov	r0, r0, LSR #LOG2PAGESIZE	@ round down to nearest page
	mov	r0, r0, LSL #LOG2PAGESIZE
	teq	r1, #0
	mcr	p15, 0, r0, c8, c6, 1		@ invalidate D TLB entry
	mcrne	p15, 0, r0, c8, c5, 1		@ invalidate I TLB entry
	mov	pc, lr

/* =============================== PageTable ============================== */

/*
 * cpu_arm1020E_set_pgd(pgd)
 *
 * Set the translation base pointer to be as described by pgd.
 *
 * pgd: new page tables
 */
	.align	5
ENTRY(cpu_arm1020E_set_pgd)
	stmfd	sp!, {lr}
	bl	cpu_arm1020E_cache_clean_invalidate_all	@ preserves r0
	mov	r1, #0
	mcr	p15, 0, r0, c2, c0, 0		@ load page table pointer
	mcr	p15, 0, r1, c8, c7, 0		@ invalidate I & D TLBs
	ldmfd	sp!, {pc}

/*
 * cpu_arm1020E_set_pmd(pmdp, pmd)
 *
 * Set a level 1 translation table entry, and clean it out of
 * any caches such that the MMUs can load it correctly.
 *
 * pmdp: pointer to PMD entry
 * pmd:  PMD value to store
 */
	.align	5
ENTRY(cpu_arm1020E_set_pmd)
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	eor	r2, r1, #0x0a			@ C & Section
	tst	r2, #0x0b
	biceq	r1, r1, #4			@ clear bufferable bit
#endif
	str	r1, [r0]
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
#endif
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain WB
	mov	pc, lr

/*
 * cpu_arm1020E_set_pte(ptep, pte)
 *
 * Set a PTE and flush it out
 */
	.align	5
ENTRY(cpu_arm1020E_set_pte)
	str	r1, [r0], #-1024		@ linux version

	eor	r1, r1, #LPTE_PRESENT | LPTE_YOUNG | LPTE_WRITE | LPTE_DIRTY

	bic	r2, r1, #0xff0
	bic	r2, r2, #3
	orr	r2, r2, #HPTE_TYPE_SMALL

	tst	r1, #LPTE_USER			@ User?
	orrne	r2, r2, #HPTE_AP_READ

	tst	r1, #LPTE_WRITE | LPTE_DIRTY	@ Write and Dirty?
	orreq	r2, r2, #HPTE_AP_WRITE

	tst	r1, #LPTE_PRESENT | LPTE_YOUNG	@ Present and Young?
	movne	r2, #0

#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	eor	r3, r2, #0x0a	@ C and Small Page?
	tst	r3, #0x0b			@ if so..
	biceq	r2, r2, #0x04	@ clear the bufferable bit
#endif
	str	r2, [r0]			@ hardware version
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
#endif
	mov	r1, #0
	mcr	p15, 0, r1, c7, c10, 4		@ drain WB
	mov	pc, lr


cpu_manu_name:
	.asciz	"ARM"
ENTRY(cpu_arm1020E_name)
	.ascii	"Arm1020E"
#ifdef CONFIG_CPU_ICACHE_ENABLE
	.ascii	"i"
#endif
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	.ascii	"d"
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	.ascii	"(wt)"
#endif
#ifdef CONFIG_CPU_DCACHE_WRITEBACK
	.ascii	"(wb)"
#endif
#endif
#ifndef CONFIG_CPU_BPREDICT_DISABLE
	.ascii	"B"
#endif
#ifdef CONFIG_CPU_CACHE_ROUND_ROBIN
	.ascii	"RR"
#endif
	.ascii	"\0"
	.align

	.section ".text.init", #alloc, #execinstr

__arm1020E_setup:
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0		@ invalidate I,D caches on v4
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer on v4
	mcr	p15, 0, r0, c8, c7, 0		@ invalidate I,D TLBs on v4
	mcr	p15, 0, r4, c2, c0, 0		@ load page table pointer
	mov	r0, #0x1f			@ Domains 0, 1 = client
	mcr	p15, 0, r0, c3, c0, 0		@ load domain access register

	mrc	p15, 0, r0, c1, c0, 0		@ Read current control register
/*
 * The only thing worth keeping from the initial control register is the endian bit
 */

	and	r0, r0, #0x0080			@ ........B....... Preserve  endian bit, zero all others.
	orr	r0, r0, #0x0070			@ .........111.... Set the SBO (Should Be One) bits
/*
 * Turn on what we want.
 */
	orr	r0, r0, #0x0001 		@ ...............M Enable MMU (Alignment is special cased elsewhere)
	orr	r0, r0, #0x0100 		@ .......S........ Enable system MMU protection
	orr	r0, r0, #0x2000			@ ..V............. Enable high vectors

#ifdef CONFIG_CPU_CACHE_ROUND_ROBIN
	orr	r0, r0, #0x4000 		@ .R.............. Force round-robin replacement
#endif

#ifndef CONFIG_CPU_BPREDICT_DISABLE
	orr	r0, r0, #0x0800 		@ ....Z........... Enable branch prediction
#endif

#ifdef CONFIG_CPU_DCACHE_ENABLE
	orr	r0, r0, #0x0004 		@ .............C.. Enable D cache
#endif
#ifndef CONFIG_CPU_WB_DISABLE
	orr	r0, r0, #0x0008 		@ ............W... Write Buffer enabled
#endif

#ifdef CONFIG_CPU_ICACHE_ENABLE
	orr	r0, r0, #0x1000 		@ ...I............ Enable I Cache
#endif

	mov	pc, lr

	.text

/*
 * Purpose : Function pointers used to access above functions - all calls
 *	     come through these
 */
	.type	arm1020E_processor_functions, #object
arm1020E_processor_functions:
	.word	cpu_arm1020E_data_abort
	.word	cpu_arm1020E_check_bugs
	.word	cpu_arm1020E_proc_init
	.word	cpu_arm1020E_proc_fin
	.word	cpu_arm1020E_reset
	.word	cpu_arm1020E_do_idle

	/* cache */
	.word	cpu_arm1020E_cache_clean_invalidate_all
	.word	cpu_arm1020E_cache_clean_invalidate_range
	.word	cpu_arm1020E_flush_ram_page

	/* dcache */
	.word	cpu_arm1020E_dcache_invalidate_range
	.word	cpu_arm1020E_dcache_clean_range
	.word	cpu_arm1020E_dcache_clean_page
	.word	cpu_arm1020E_dcache_clean_entry

	/* icache */
	.word	cpu_arm1020E_icache_invalidate_range
	.word	cpu_arm1020E_icache_invalidate_page

	/* tlb */
	.word	cpu_arm1020E_tlb_invalidate_all
	.word	cpu_arm1020E_tlb_invalidate_range
	.word	cpu_arm1020E_tlb_invalidate_page

	/* pgtable */
	.word	cpu_arm1020E_set_pgd
	.word	cpu_arm1020E_set_pmd
	.word	cpu_arm1020E_set_pte
	.size	arm1020E_processor_functions, . - arm1020E_processor_functions

	.type	cpu_arm1020E_info, #object
cpu_arm1020E_info:
	.long	cpu_manu_name
	.long	cpu_arm1020E_name
	.size	cpu_arm1020E_info, . - cpu_arm1020E_info

	.type	cpu_arch_name, #object
cpu_arch_name:
	.asciz	"armv5te"
	.size	cpu_arch_name, . - cpu_arch_name

	.type	cpu_elf_name, #object
cpu_elf_name:
	.asciz	"v5"
	.size	cpu_elf_name, . - cpu_elf_name
	.align

	.section ".proc.info", #alloc, #execinstr

	.type	__arm1020E_proc_info,#object
__arm1020E_proc_info:
	.long	0x4105a200			@ ARM 1020E (Architecture v5TE)
	.long	0xff0ffff0
	.long	0x00000c1e			@ mmuflags
	b	__arm1020E_setup
	.long	cpu_arch_name
	.long	cpu_elf_name
	.long	HWCAP_SWP | HWCAP_HALF | HWCAP_THUMB
	.long	cpu_arm1020E_info
	.long	arm1020E_processor_functions
	.size	__arm1020E_proc_info, . - __arm1020E_proc_info

